# Architecture

This document describes the main parts of the project, what each class
is responsible for, and how it connects with others.

## Overview

The project is split into three namespaces:

- **`Storage`** - the data layer. Owns the keys and values, tracks memory, TTL. Knows nothing about commands or input/output.
- **`Commands`** - the command layer. One command = one class (inhereted from interface). Each command reads its arguments, talks to `Storage`, and writes a reply.
- **`Parser`** - the input/output layer. Turns a raw line into tokens, and contains interface for response.
- **`Application`** - carries out the REPL work. Have own inherited class for respinses, parses valkey launch command

The `main` works with **`Application`**.

Dependencies go one way:

    main -> Application -> Parser -> Commands -> Storage

---

## Namespace `Storage`

### `ValueVariant` (header `ValueVariant.hpp`)

Set of type aliases that define what a value can be.

**Atomic Aliases:**

- `StringType = std::string` - a plain string.
- `ListType = std::deque<std::string>` - an ordered list. Deque because we push and pop from both ends.
- `SetType = std::unordered_set<std::string>` - a set of unique strings.
- `GeoType = std::vector<GeoPoint>` - a list of geo points.
- `GeoPoint = {double lon, double lat, std::string mamber}` - lon, lat and member name.

**Alias `ValueVariant`:**

    std::variant<StringType, ListType, SetType, GeoType>

This is the single source of info about which value types the database
supports. Adding a new type is adding it here.

**Responsibility:** define the types.

**Not responsible for:** any operations on the values. Those live in
commands and in `Entry`.

---

### `Entry` (header `Entry.hpp`)

One record in the database: a value plus metadata.

**Fields (private):**

- `size_t memory_usage_` - an approximate size of this entry object in bytes.

**Fields (public):**

- `ValueVariant value` - the data.
- `std::optional<t_point> exp_time` - time remaining until key expire. `nullopt`
  means no TTL for key.
- `t_point` is `std::chrono::system_clock::time_point`.

**Methods:**

- `Entry(ValueVariant val, std::optional<t_point> expire = nullopt)` - constructor,computes `memory_usage_` with `CalculateSelfSize()`.
- `bool IsExpired() const noexcept` - is this entry expired by TTL.
- `size_t MemoryUsage() const noexcept` - getter for `memory_usage_`.
- `size_t CalculateSelfSize() const noexcept` - approximate size of the entry.

**Responsibility:** includes value and TTL.

**Not responsible for:**

- Removing itself from the storage when expired. `StorageEngine` deletes it.
- Knowing its key. Key holds in the map.

**Design note:** `Entry` is immutable after construction.
Mutating commands like `APPEND` or `LSET` build a new `Entry` and
replace the old one. This makes MT easier.

---

### `MemoryManager` (header `MemoryManager.hpp`)

Tracks how much memory the whole storage uses, and enforces an optional
limit.

**Fields:**

- `size_t actual_usage_` - bytes used right now.
- `size_t usage_limit_` - the limit. `0` means "no limit".

**Constants:** `KB`, `MB`, `GB` - for parsing size suffixes.

**Methods:**

- `MemoryManager(size_t limit_bytes = 0)` - constructor.
- `size_t GetUsage() const noexcept` - current usage.
- `size_t GetLimit() const noexcept` - current limit.
- `void SetLimit(size_t bytes)` - used by `CONFIG SET maxmemory`.
- `bool CanAllocate(size_t new_size, size_t old_size = 0) const noexcept` - pure check: "Can we replace a block of `old_size` with a block of `new_size`.
- `void Resize(size_t old_size, size_t new_size)` - check and apply. Throws exception if we cant resize.
- `static size_t ParseSizeToBytes(const std::string& str)` - parses strings like `"64kb"`, `"2gb"`, `"1024"`.

**Responsibility:** one number for the whole process.

**Not responsible for:**

- Knowing what the memory is used for. No keys, no values.
- Without MT.

---

### `StorageEngine` (header `StorageEngine.hpp`)

The main data structure: a map from keys to entries, plus the rules
for TTL and memory.

**Fields:**

- `std::unordered_map<std::string, Entry> storage` - the data.
- `MemoryManager& mem_manager` — a reference. The manager lives longer
  than the engine.

**Methods:**

- `StorageEngine(MemoryManager& mm)` - constructor.
- `std::shared_ptr<const Entry> Get(const std::string& key)` - looks up a key. Returns `nullptr` if not found or expired.
- `void Set(const std::string& key, Entry&& entry)` - writes an entry. Recalculates memory and calls `mem_manager.Resize`.
- `bool Do(const std::string& key, F&& f)` - executing the lambda passed from the command.
- `void DoMulti(F&& f, const Keys&... keys)` - executing the multikey lambda passed from command.
- `int Remove(const std::vector<std::string>& keys)` - deletes keys, returns how many were removed.
- `bool Exist(const std::string& key)` - checks existence of keys. Expired keys count as missing.
- `std::vector<std::string> GetAllKeys(const std::string& pattern) const` - returns keys matching a glob pattern.
- `void Flush()` - removes everything.
- `size_t Size() const` - number of keys.
- `size_t GetMemUsage(const std::string& key)` - size of one key-value.
- `size_t TotalMemory() const` - total usage.
- `size_t GetMaxMemory() const` - getter for memory limit.
- `void SetMaxMemory(size_t bytes)` - setter for max memory.
- `size_t RealTotal() const` - real total memory for invariant check.

**Private helper:**

- `ConstIterator FindLiveShared(const std::string& key) const` - finds key.
- `Iterator FindLiveUnique(const std::string& key)` - finds or deletes key.
- `check_ttl(key)` - looks at `exp_time`, removes the entry if expired,
  and updates `MemoryManager`.

**Responsibility:**

- Own the map. Nobody else touches `Storage` directly.
- Checks TTL rule on every access.
- Keep `MemoryManager` in sync on every mutation.

**Not responsible for:**

- Parsing commands or arguments.
- Formatting replies.
- Knowing the type of a value. Contains `Entry`.

---

## Namespace `Commands`

### `Context` (header `Context.hpp`)

A small "package" passed to a command. Holds everything a command
needs to do its job.

**Fields:**

- `std::vector<std::string> args` - the arguments of the command, without the command name (indexes from `0`).
- `StorageEngine& storage` - reference to the storage.
- `MemoryManager& mem` - reference to the memory manager
- `IResponse& out` - reference to the reply formatter.

**Helper methods for argument parsing** (planned):

- `int AsInt(size_t index) const`
- `double AsDouble(size_t index) const`
- `std::string_view AsString(size_t index) const`
- `Unit AsUnit(size_t index) const` - for `m`, `km`, `mi`, `ft`.

**Responsibility:** carry data

**Not responsible for:**

- Parsing a full line. `Tokenizer` does that.
- Storing any ctx data between calls. A `Context` obj is created for every line.

---

### `ICommand` (header `ICommand.hpp`)

Abstract base class for all commands.

**Methods:**

- `virtual ~ICommand() = default;` - required.
- `virtual void Execute(Context& ctx) = 0;` - runs the command.

**Responsibility:** define one operation.

**Not responsible for:**

- Knowing who called it.
- Registering itself.

---

### `CommandReg` (header `CommandReg.hpp`)

The command registry. A map command names - command objects.

**Fields:**

- `std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_` - keys are stored in UPPER CASE.

**Methods:**

- `void RegisterCommand(const std::string& name, std::unique_ptr<ICommand> cmd)` - adds a command. The name is upper-cased first.
- `ICommand* FindCommand(const std::string& name) const` - returns the command or `nullptr`.

**Responsibility:** holds commads list (umap).

**Not responsible for:**

- Running commands.
- Parsing input.

---

### `RegisterAll fuction` (header `RegisterAll.hpp`)

- `void RegisterAll()` - one place where all commands are created and registered, called once at launch.

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
  `SET key "hello world"`), we add a `case '"'` realization.

**Responsibility:** cut a string into pieces. Nothing more.

**Not responsible for:**

- Understanding commands.
- Converting num-string to an integer.
- Handling `[`, `]`, `|` - those are documentation syntax, not input.

---

### `IResponse` / `ResponseFormatter` (header `ResponseFormatter.hpp`)

A single place for all output formatting.

**Interface:**

- `void Ok()` - prints `OK`.
- `void Nil()` - prints `(nil)` for `no answer`.
- `void Int(long long n)` - prints an integer.
- `void Bulk(const std::string& s)` - prints a single string.
- `void Array(const std::vector<std::string>& items)` - prints a list of strings.
- `void Error(const std::string& msg)` - prints an error to `stderr`.

**Responsibility:** know the exact output format. If the format changes
(for example, we switch to RESP), only this class needed changes.

**Not responsible for:**

- Building data.
- Knowing specific commands.

---

## Example Data flow

**`SET key value`**

1. `Application` reads a line.
2. `Tokenizer` returns `{"command", "key", "value"}`.
3. `CommandReg::Find("command")` returns a `SomeCommand*`.
4. `Application` builds a `Context` with args `{"key", "value"}` and runs
   `SOmeCommand::Execute(ctx)`.
5. The command builds a lambda and bring it to `Do`/`DoMulty` method.
6. `StorageEngine::Set` asks `MemoryManager::Resize` if it fits.
7. On success, `ctx.out.Ok()` writes `OK` to stdout.
8. On failure, `MemoryManager` throws and `main` prints the OOM error to stderr.

**Unknown command**

1. `CommandReg::Find` returns `nullptr`.
2. `main` writes an error to stderr and continues.

**OOM**

1. `MemoryManager::Resize` throws.
2. `StorageEngine::Set` lets the exception through.
3. `Repl` catches it and writes
   `(error) OOM command not allowed when used memory > 'maxmemory'`
   to stderr.

---

## Rules to keep in mind

1. **One direction of dependencies.** `Storage` never includes `Commands`
2. **No globals.** `StorageEngine`, `MemoryManager`, and the response formatter are given through `Context`
3. **Commands have no state.** The same `SetCommand` object may serve many calls, possibly from different threads later
4. **`Context` is per call.** It is created inside `main` for each line and destroyed after.
5. **`Entry` is a value.** Do not store pointers to it.
6. **All mutations go through `StorageEngine`.** No code outside `StorageEngine` touches the map. Access to `MemoryManger` happends under mutex from `StorageEngine`. 
7. **All memory accounting goes through `MemoryManager`.** No code outside updates `actual_usage_`.
8. **All output goes through the formatter.** No `std::cout` in commands, only `IResponse` interface.
