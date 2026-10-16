# Design Decisions

Short list of decisions. Newest on top. Numbers are permanent.

---

## MT strategy — one shared_mutex inside StorageEngine

**Decision**

`StorageEngine` owns a single `shared_mutex mtx_`. It protects
both `storage_` and `mem_manager_`. Readers under `shared_lock`, writers
under `unique_lock`. `MemoryManager` has no lock of its own, because every
access to it goes through `StorageEngine`.

**Why**

- Simple model. No lock ordering.
- `Entry` is immutable and `Get` returns `shared_ptr<const Entry>`,
  so readers never see dead values.

**Consequences**

- Commands must not call `MemoryManager` directly; use
  `StorageEngine::SetMaxMemory` / `GetMaxMemory`.
- `Size`, `GetAllKeysByPattern`, `Flush`, `Remove`, `Set`, `Do`,
  `DoMulti` take `unique_lock`.
- `Get`, `Exist`, `GetMemUsage`, `TotalMemory`, `GetMaxMemory` take
  `shared_lock`.

---

## Mutating commands goes through Do / DoMulti

**Decision**

Mutating commands do not call `Get` + `Set` any longer. They call
`StorageEngine::Do(key, fn)`, or `DoMulti(fn, keys...)` for
several. Both take a `unique_lock` for the whole read-modify-write.

**Why**

- `Get` + `Set` is not atomic: between them another thread can change
  the data.
- Simple extensibility.

**Consequences**

- `SMOVE` and `GEOSEARCHSTORE` use `DoMulti` with two distinct keys.
- Lambdas passed to `Do` / `DoMulti` must not touch `storage` or write
  the reply. They only read `current`.

---

## Commands and Context are stateless

**Decision**

A command object has no mutable member state. All per-call data lives
in `Context`, created fresh per line and destroyed after `Execute`.

**Why**

- MT. One command instance can serve many threads safely.

**Consequences**

- No member variables in command classes.
- `main/resp` does not reset anything between lines.

---

## Response formatter is not a singleton

**Decision**

`IResponse` is passed to commands through `Context`.

**Why**

- Each TCP connection needs its own output buffer.
- Easy to test: fake response formatter.

**Consequences**

- Commands never touch outstreams.
- `Context` holds `IResponse&`.

---

## SetType is std::unordered_set<std::string>

**Decision**

Order in `SMEMBERS` / `SUNION` / `SINTER` / `SDIFF` is not guaranteed.

**Why**

- valkey.io docs say the order is unspecified, so thats the best option.

**Consequences**

- Set tests compare as a set, not as a list.

---

## Get returns std::shared_ptr<const Entry>

**Decision**

Map is `unordered_map<string, shared_ptr<const Entry>>`. `Entry` is
immutable, mutations go through copy-on-write.

**Why**

- Safe under MT: readers keep the entry alive after removal.
- No copies of the value, only an atomic refcount.

**Consequences**

- No `Entry*` / `Entry&` leaks from `StorageEngine`.
- `Set` allocates a `shared_ptr` per write.