# Architecture

This document describes the main parts of the project, what each class
is responsible for, and how they talk to each other.

## Overview

The project is split into three namespaces:

- **`Storage`** — the data layer. Owns the keys and values, tracks memory,
  handles TTL. Knows nothing about commands or input/output.
- **`Commands`** — the command layer. One command = one class. Each command
  reads its arguments, talks to `Storage`, and writes a reply.
- **`Parser`** — the input/output layer. Turns a raw line into tokens, and
  turns command replies into text for the user.

The `main` function (the REPL) glues everything together: reads a line,
splits it, looks up a command, runs it, prints the result.

Dependencies go one way:

    main → Parser → Commands → Storage

`Storage` does not know about `Commands`. `Commands` does not know about
`main` or the console. This keeps the core testable without any I/O.

---

## Namespace `Storage`

### `ValueVariant` (header `ValueVariant.hpp`)

Not a class, but a set of type aliases that define what a value can be.

**Aliases:**

- `StringType = std::string` — a plain string.
- `ListType = std::deque<std::string>` — an ordered list. Deque because
  we push and pop from both ends.
- `SetType = std::unordered_set<std::string>` — a set of unique strings.
- `GeoType = std::vector<GeoPoint>` — a list of geo points.

**Struct `GeoPoint`:**

- `double longitude`
- `double latitude`
- `std::string member`

**Alias `ValueVariant`:**

    std::variant<StringType, ListType, SetType, GeoType>

This is the single source of truth about which value types the database
supports. Adding a new type means adding it here, and then the compiler
will tell us every place that needs updating.

**Responsibility:** define the types. No logic.

**Not responsible for:** any operations on the values. Those live in
commands and in `Entry`.

---

### `Entry` (header `Entry.hpp`)

One record in the database: a value plus metadata.

**Fields (public):**

- `ValueVariant value` — the actual data.
- `std::optional<t_point> exp_time` — when the key expires. `nullopt`
  means "no TTL". `t_point` is `std::chrono::system_clock::time_point`.

**Fields (private):**

- `size_t memory_usage_` — an approximate size of this entry in bytes,
  not counting the key.

**Methods:**

- `Entry(ValueVariant val, std::optional<t_point> expire = nullopt)` —
  constructor. Computes `memory_usage_` via `CalculateSelfSize()`.
- `bool IsExpired() const noexcept` — true if the TTL has passed.
- `size_t MemoryUsage() const noexcept` — getter for `memory_usage_`.
- `size_t CalculateSelfSize() const noexcept` — approximate size of the
  entry. Uses `std::visit` over the variant.

**Responsibility:** hold a value and its TTL. Nothing else.

**Not responsible for:**

- Removing itself from the storage when expired. `StorageEngine` does that.
- Knowing its key. The key is stored in the map, not in the entry.
- Formatting itself for output.
- Being thread-safe. It is a value type — copy it, move it, and don't
  share a single instance between threads.

**Design note:** `Entry` is meant to be immutable after construction.
Mutating commands like `APPEND` or `LSET` build a *new* `Entry` and
replace the old one. This makes future multi-threading much easier:
readers keep a `shared_ptr<const Entry>` and are never disturbed.

---

### `MemoryManager` (header `MemoryManager.hpp`)

Tracks how much memory the whole storage uses, and enforces an optional
limit.

**Fields:**

- `size_t actual_usage_` — bytes used right now.
- `size_t usage_limit_` — the limit. `0` means "no limit".

**Constants:** `KB`, `MB`, `GB` — for parsing size suffixes.

**Methods:**

- `MemoryManager(size_t limit_bytes = 0)` — constructor.
- `size_t GetUsage() const noexcept` — current usage.
- `size_t GetLimit() const noexcept` — current limit.
- `void SetLimit(size_t bytes)` — used by `CONFIG SET maxmemory`.
  Throws if the new limit is below current usage.
- `bool CanAllocate(size_t new_size, size_t old_size = 0) const noexcept` —
  pure check: "would replacing a block of `old_size` with a block of
  `new_size` still fit?"
- `void Resize(size_t old_size, size_t new_size)` — check and apply.
  Throws if it does not fit.
- `static size_t ParseSizeToBytes(const std::string& str)` — parses
  strings like `"64kb"`, `"2gb"`, `"1024"`.

**Responsibility:** one number for the whole process. Nothing more.

**Not responsible for:**

- Knowing what the memory is used for. No keys, no values.
- Building error messages. It throws; the caller formats the text.
- Being thread-safe yet. When we add multi-threading, `actual_usage_`
  becomes `std::atomic<size_t>` and `Resize` becomes one atomic step.
  The API will not change.

---

### `StorageEngine` (header `StorageEngine.hpp`)

The main data structure: a map from keys to entries, plus the rules
for TTL and memory.

**Fields:**

- `std::unordered_map<std::string, Entry> storage` — the data.
  *(May become `unordered_map<string, shared_ptr<const Entry>>` later —
  see `DECISIONS.md`.)*
- `MemoryManager& mem_manager` — a reference. The manager lives longer
  than the engine.

**Methods:**

- `StorageEngine(MemoryManager& mm)` — constructor.
- `Entry* Get(const std::string& key)` — looks up a key. Returns
  `nullptr` if not found or expired. *(Signature under review —
  see `DECISIONS.md`.)*
- `void Set(const std::string& key, Entry&& entry)` — writes an entry.
  Recalculates memory and calls `mem_manager.Resize`. Throws on OOM.
- `int Remove(const std::vector<std::string>& keys)` — deletes keys,
  returns how many were actually removed.
- `bool Exist(const std::string& key)` — checks existence. Expired keys
  count as missing.
- `std::vector<std::string> GetAllKeys(const std::string& pattern) const` —
  returns keys matching a glob pattern. Used by `KEYS`.
- `void Flush()` — removes everything.
- `size_t Size() const` — number of keys.
- `size_t GetMemUsage(const std::string& key)` — size of one key+value.
- `size_t TotalMemory() const` — total usage. Usually obtained from
  `MemoryManager`, but kept here for convenience.

**Private helper:**

- `check_ttl(key)` — looks at `exp_time`, removes the entry if expired,
  and updates `MemoryManager`. Called from every public method that
  reads or writes a key, so expired entries never leak into results.

**Responsibility:**

- Own the map. Nobody else touches `storage` directly.
- Apply TTL rules on every access.
- Keep `MemoryManager` in sync on every mutation.

**Not responsible for:**

- Parsing commands or arguments.
- Formatting replies.
- Knowing the type of a value. From its point of view, everything is
  an `Entry`.

---

## Namespace `Commands`

### `Context` (header `Context.hpp`)

A small "package" passed to a command. Holds everything a command
needs to do its job.

**Fields:**

- `std::vector<std::string> args` — the arguments of the command,
  without the command name.
- `StorageEngine& storage` — reference to the storage.
- `MemoryManager& mem` — reference to the memory manager. Needed by
  `CONFIG SET/GET` and `MEMORY USAGE`.
- `IResponse& out` — reference to the reply formatter.

**Helper methods for argument parsing** (planned):

- `int AsInt(size_t index) const`
- `double AsDouble(size_t index) const`
- `std::string_view AsString(size_t index) const`
- `Unit AsUnit(size_t index) const` — for `m`, `km`, `mi`, `ft`.

**Responsibility:** carry data. Nothing else.

**Not responsible for:**

- Parsing a full line. `Tokenizer` does that.
- Storing any state between calls. A new `Context` is created for every
  line the user types.

---

### `ICommand` (header `ICommand.hpp`)

Abstract base class for all commands.

**Methods:**

- `virtual ~ICommand() = default;` — required, since instances are stored
  in `unique_ptr<ICommand>`.
- `virtual void Execute(Context& ctx) = 0;` — runs the command.

**Responsibility:** define one operation.

**Not responsible for:**

- Knowing who called it.
- Registering itself.

**Design note:** each command is a separate class (for example
`SetCommand`, `GetCommand`, `LPushCommand`). This is easy to test and
easy to read, at the cost of many small files.

---

### `CommandReg` (header `CommandReg.hpp`)

The command registry. A map from command names to command objects.

**Fields:**

- `std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_`
  — keys are stored in UPPER CASE for case-insensitive lookup.

**Methods:**

- `void Register(const std::string& name, std::unique_ptr<ICommand> cmd)`
  — adds a command. The name is upper-cased first.
- `ICommand* Find(const std::string& name) const` — returns the command
  or `nullptr`.
- `void RegisterAll()` — one place where all commands are created and
  registered. Called once at startup.

**Responsibility:** know which command exists. Nothing else.

**Not responsible for:**

- Running commands.
- Parsing input.
- Knowing about `StorageEngine`.

---

## Namespace `Parser`

### `Tokenizer` (header `Tokenizer.hpp`)

Splits one input line into tokens.

**Method:**

- `std::vector<std::string> operator()(const std::string& line) const`

**Rules:**

- Whitespace separates tokens (`' '`, `'\t'`, `'\n'`, `'\r'`, `'\v'`, `'\f'`).
- Empty tokens are dropped.
- An empty line or a line of spaces returns an empty vector.
- Quotes are **not** supported yet. If we need them (for example
  `SET key "hello world"`), we add a `case '"'` branch.

**Responsibility:** cut a string into pieces. Nothing more.

**Not responsible for:**

- Understanding commands.
- Converting `"123"` to an integer.
- Handling `[`, `]`, `|` — those are documentation syntax, not input.

---

### `IResponse` / `ResponseFormatter` (header `ResponseFormatter.hpp`, planned)

A single place for all output formatting.

**Interface (draft):**

- `void Ok()` — prints `OK`.
- `void Nil()` — prints `(nil)` for a missing key.
- `void Int(long long n)` — prints an integer reply.
- `void Bulk(const std::string& s)` — prints a single string reply.
- `void Array(const std::vector<std::string>& items)` — prints a list
  of strings.
- `void Error(const std::string& msg)` — prints an error to `stderr`.

**Responsibility:** know the exact output format. If the format changes
(for example, we switch to RESP), only this class changes.

**Not responsible for:**

- Building data.
- Knowing specific commands.

**Design note:** the formatter must **not** be a global singleton.
In the single-threaded REPL there is one instance. Later, when the
server handles many connections, each connection gets its own formatter
with its own output buffer.

---

## Data flow

**`SET key value`**

1. `main` reads a line.
2. `Tokenizer` returns `{"SET", "key", "value"}`.
3. `CommandReg::Find("SET")` returns a `SetCommand*`.
4. `main` builds a `Context` with args `{"key", "value"}` and runs
   `SetCommand::Execute(ctx)`.
5. The command builds a new `Entry` with a `StringType` value.
6. `StorageEngine::Set` asks `MemoryManager::Resize` if it fits.
7. On success, `ctx.out.Ok()` writes `OK` to stdout.
8. On failure, `MemoryManager` throws and `main` prints the OOM error
   to stderr.

**`GET key`**

Same until step 4. Then:

5. `StorageEngine::Get` checks TTL, removes the key if expired, and
   returns `nullptr`.
6. If `nullptr`, the command calls `ctx.out.Nil()`.
7. Otherwise `ctx.out.Bulk(value)`.

**Unknown command**

1. `CommandReg::Find` returns `nullptr`.
2. `main` writes an error to stderr and continues.

**OOM**

1. `MemoryManager::Resize` throws.
2. `StorageEngine::Set` lets the exception through (or re-wraps it).
3. `main` catches it and writes
   `(error) OOM command not allowed when used memory > 'maxmemory'`
   to stderr.

---

## Rules to keep in mind

1. **One direction of dependencies.** `Storage` never includes
   `Commands`. `Commands` never includes `main`.
2. **No globals.** `StorageEngine`, `MemoryManager`, and the response
   formatter are passed explicitly through `Context`.
3. **Commands have no state.** The same `SetCommand` object may serve
   many calls, possibly from different threads later.
4. **`Context` is per call.** It is created inside `main` for each line
   and destroyed after.
5. **`Entry` is a value.** Do not store pointers to it. Copy or move it.
6. **All mutations go through `StorageEngine`.** No code outside
   `StorageEngine` touches the map.
7. **All memory accounting goes through `MemoryManager`.** No code
   outside updates `actual_usage_`.
8. **All output goes through the formatter.** No `std::cout` in commands.