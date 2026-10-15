# Contracts

Invariants, pre- and post-conditions for the core classes.

---

## `Storage::MemoryManager`

### Invariants

- `usage_limit_ == 0` means "no limit".
- `actual_usage_ <= usage_limit_` whenever `usage_limit_ != 0`.

### Thread safety

- Not protected by its own mutex.
- All access goes through `StorageEngine` under `mtx_`.
- Reads are taken under `shared_lock`.
- Mutations are taken under `unique_lock`.

### Pre

- `Resize(old, new)`: `old <= actual_usage_`.
- `SetLimit(bytes)`: `bytes == 0 || bytes >= actual_usage_`.

### Post

- `Resize(old, new)`: `actual_usage' = actual_usage - old + new`.
- On OOM: throws `OOMException`, `actual_usage_` unchanged.
- On invalid limit: throws `std::invalid_argument`.

---

## `Storage::Entry`

### Invariants

- `memory_usage_ == CalculateSelfSize()`.
- Immutable after construction; mutations go through copy-on-write.
- Never mutated through `shared_ptr<const Entry>`.

### Pre

- `WithValue(new)`: `value` holds any of the variant alternatives.

### Post

- `WithValue(new)`: returns a new `Entry` with the new value and the
  same `exp_time`.

---

## `Storage::StorageEngine`

### Invariants

- `storage_` is the only place keys and entries live.
- `mem_manager_.GetUsage() == sum of TotalSizeFor(key, entry)` over all live entries.
- No `Entry*` or `Entry&` leaks outside this class.
- Expired entries are never visible to callers.
- `mtx_` protects both `storage_` and `mem_manager_`.

### Pre

- `Get`, `Exist`, `Set`, `Remove`, `GetMemUsage`, `Do`: key may or may not exist.
- `Set`, `Do`: entry was constructed by the caller.
- `DoMulti`: the passed keys are pairwise distinct.
- `SetMaxMemory(bytes)`: `bytes == 0 || bytes >= current usage`.

### Post

- `Get`: returns `nullptr` if the key is missing or expired.
- `Set`, `Do`, `DoMulti`: strong exception guarantee. On `OOMException`
  or on a throw from the lambda, `storage_` or `mem_manager_`  isn't modified.
- `Remove`: returns the number of keys actually removed.
- `Flush`: `storage_` empty, `GetUsage() == 0`.
- `SetMaxMemory`: throws `std::invalid_argument` if `bytes < current usage`.

---

## `Commands::Context`

### Invariants

- Created per call, destroyed after `Execute`.
- References outlive the `Context`.
- No mutable state between calls.
- `Memory()` is exposed, but commands must not call `MemoryManager`
  methods directly — only through `StorageEngine`.

### Pre

- `GetArgumentAs*`: `i` in range.

### Post

- `GetArgumentAsLLInt`, `GetArgumentAsDouble`: throw `CommandError` on
  invalid input, never return garbage.

---

## `Commands::ICommand`

### Invariants

- Stateless between calls.
- Never touches outstreams directly.

### Pre

- `ctx` holds valid references.

### Post

- On success: exactly one reply via `ctx.Out()`.
- On failure: throws exception.

---

## `Application::Repl`

### Invariants

- Reads lines from one stream, writes replies through one `IResponse`.
- Prints nothing on its own.

### Post

- Catches `CommandError`, `OOMException`, `std::exception` and forwards
  them via `out.Error(...)`.