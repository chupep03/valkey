# Design Decisions

Short log of decisions. Goal: not re-deciding the same thing every week.
Newest on top. Numbers are permanent — do not renumber.

---

## D-003: OOM is a dedicated `OOMException`

**Status:** proposed

**Decision**

Errors from exceeding `maxmemory` throw a dedicated `OOMException`
(derived from `std::runtime_error`) with no spec text inside. The REPL
catches it separately and prints the exact string from the spec:

    (error) OOM command not allowed when used memory > 'maxmemory'

**Why**

- The spec text lives in one place (the formatter / `main`), not in
  ten files.
- The REPL can tell OOM apart from other `runtime_error`s by type,
  without string matching.
- Fits the existing style (we already throw on bad input and broken
  invariants).

**Consequences**

- New file `Storage/OOMException.hpp`.
- `MemoryManager::Resize` throws `OOMException`, not `std::runtime_error`.
- `StorageEngine::Set` lets it through unchanged.
- `main` has one extra `catch (const OOMException&)`.

---

## D-002: `SetType` is `std::unordered_set<std::string>`

**Status:** proposed

**Decision**

The value type for sets is `std::unordered_set<std::string>`.
Order of returned elements in `SMEMBERS` / `SUNION` / `SINTER` /
`SDIFF` is not guaranteed.

**Why**

- valkey.io docs say the order is unspecified for these commands.
- Simplest option: no extra bookkeeping.
- If tests turn out to require order, switching to insertion-ordered
  is a local change in `ValueVariant.hpp` and the Set commands.

**Consequences**

- Set-command tests must compare as a set, not as a list.
- If a specific order is later required, see the future insertion-
  ordered option (not written yet).

---

## D-001: `StorageEngine::Get` returns `std::shared_ptr<const Entry>`

**Status:** proposed

### Decision

`Get` returns `std::shared_ptr<const Entry>` (or `nullptr` if the key
is missing or expired). The internal map becomes:

    std::unordered_map<std::string, std::shared_ptr<const Entry>>

`Entry` is immutable after construction. All mutations go through
copy-on-write: build a new `Entry`, call `Set`, let the old one die
when readers release it.

### Why

- Safe under multi-threading: readers keep the entry alive even if the
  map removes it.
- No copies of the value itself — only an atomic refcount.
- Copy-on-write falls out naturally; readers and writers will not
  block each other once we add `std::shared_mutex`.
- Only option that will not need rewriting when we add MT.

### Consequences

- `Entry*` and `Entry&` never leak out of `StorageEngine` again.
- Commands that used to mutate in place now build a new `Entry` and
  `Set` it. Slightly more code, no aliasing bugs.
- `Set` allocates a `shared_ptr` per write.
- `Entry` must stay immutable after construction.

---

## D-004: TTL uses `std::chrono::system_clock`

**Status:** accepted

### Decision

`exp_time` is `std::optional<std::chrono::system_clock::time_point>`.

### Why

- TTL is defined in real wall-clock seconds, like in Redis / valkey.
- `steady_clock` measures a duration since boot, not wall time.
  Wrong tool for "expire in 60 seconds".

### Consequences

- If the OS clock jumps, some keys may expire sooner or later than
  expected. Same behavior as Redis. Acceptable.

---

## D-005: Response formatter is not a singleton

**Status:** accepted

### Decision**

`IResponse` is passed to commands through `Context`. No global
`std::cout` access inside commands, no `ResponseFormatter::instance()`.

### Why

- We plan a TCP frontend with many connections. Each connection needs
  its own output buffer.
- Easy to test: pass a fake formatter that records strings.

### Consequences

- `Context` holds `IResponse& out`.
- Commands never touch `std::cout` directly.
- `main` (or the future server) creates one formatter per session.

---

## D-006: Commands and `Context` are stateless between calls

**Status:** accepted

### Decision

A command object has no mutable member state. Everything per call goes
through `Context`, which is created fresh for each input line and
destroyed after.

### Why

- Multi-threading is on the roadmap. Stateless commands can be reused
  by many threads safely.
- Easier to test and reason about.

### Consequences

- No member variables in command classes (compile-time constants are
  fine).
- `main` does not need to reset anything between lines.

---

## D-007: The tokenizer does not support quotes (for now)

**Status:** accepted

### Decision

The tokenizer splits on whitespace only. `"..."` and `'...'` are not
special. `SET key "hello world"` produces four tokens.

### Why

- Spec does not require quotes.
- valkey's own text protocol does not use them either (only
  `valkey-cli`, on the client side).
- Adding them later is one `case '"'` branch in the tokenizer.

### Consequences

- Values with spaces are not supported right now.
- If needed, the change is local to `Parser/Tokenizer.hpp`.

---
