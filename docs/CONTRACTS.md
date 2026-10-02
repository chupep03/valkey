# Contracts

Invariants and pre/post conditions for the core classes.

## `Storage::MemoryManager`

### Invariants

- `usage_limit_ == 0` means no limit.
- `actual_usage_ == Σ TotalSizeFor(key, entry)` over all live keys
- `actual_usage_ <= usage_limit_` whenever except no usage limit `usage_limit == 0`.

### Pre

- `Resize(old, new)`: `old <= actual_usage_`.
- `SetLimit(bytes)`: `bytes == 0 || bytes >= actual_usage_`.

### Post

- `Resize(old, new)`: `actual_usage' = actual_usage - old + new`.
- On OOM, throws `OOMException` and leaves `actual_usage_` unchanged.

## `Storage::Entry`

### Invariants

- `memory_usage_ == CalculateSelfSize()`
- Immutable after construction. Copy-on-write only
- Never mutated through `shared_ptr<const Entry>`

### Pre

- `WithValue(new)`: `value` currently holds any alternative.

### Post

- `WithValue(new)`: returns a new `Entry` with `new` value and the
  same `exp_time`.

## `Storage::StorageEngine`

### Invariants

- `storage_` is the only place keys and entries live.
- `mem_manager_.GetUsage() == Σ over storage_ TotalSizeFor(key, entry)`.
- No `Entry*` or `Entry&` leaks outside this class.
- Expired entries are never visible to callers.

### Pre

- `Get/Exist/Set/Remove/GetMemUsage`: `key` may or may not exist.
- `Set`: `entry` was constructed by the caller.

### Post

- `Get`: returns `nullptr` if missing or expired.
- `Set`: strong exception guarantee — on `OOMException`, nothing changes.
- `Remove`: returns the number of keys actually removed.
- `Flush`: `storage_` empty, `mem_manager_.GetUsage() == 0`.

## `Commands::Context`

### Invariants

- Created per call, destroyed after `Execute`.
- References outlive the context.
- No mutable state carried between calls.

### Pre

- `GetArgumentAs*`: `i` is expected to be valid; out-of-range throws.
- `RequireArgs(n)` / `RequireMinArgs(n)`: none.

### Post

- `GetArgumentAsLLInt` / `GetArgumentAsDouble`: throws `CommandError`
  on invalid input, never returns garbage.

## `Commands::ICommand`

### Invariants

- Stateless between calls (D-006).
- Never touches `std::cout` / `std::cerr` directly.

### Pre

- `ctx` holds valid references.

### Post

- On success: writes exactly one reply via `ctx.Out()`.
- On failure: throws `CommandError`, `WrongType`, or `OOMException`.

## `Application::Repl`

### Invariants

- Reads lines from one stream, writes replies through one `IResponse`.
- Prints nothing on its own.
- Exits on `EXIT` (case-insensitive) or EOF.

### Post

- Catches `CommandError`, `OOMException`, `std::exception` and forwards
  them as `out.Error(...)` — does not propagate to `main`.