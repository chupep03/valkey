# Design Decisions

Short log of decisions. Goal: not re-deciding the same thing every week.
Newest on top. Numbers are permanent — do not renumber.

---

## `SetType` is `std::unordered_set<std::string>`

**Decision**

The value type for sets is `std::unordered_set<std::string>`.
Order of returned elements in `SMEMBERS` / `SUNION` / `SINTER` /
`SDIFF` is not guaranteed.

**Why**

- valkey.io docs say the order is unspecified for these commands.

**Consequences**

- Set-command tests must compare as a set, not as a list.
- If a specific order is later required, new type may be added.

---

## `StorageEngine::Get` returns `std::shared_ptr<const Entry>`

### Decision

`Get` returns `std::shared_ptr<const Entry>` (or `nullptr` if the key
is missing or expired). The internal map becomes:

    std::unordered_map<std::string, std::shared_ptr<const Entry>>

`Entry` is immutable after construction. All mutations go through
copy-on-write: build a new `Entry`, call `Set`, let the old one destruct.

### Why

- Safe under multi-threading: readers keep the entry alive and immutable
- No copies of the value itself - only an atomic refcount.
- Only option that will not need rewriting when MT will be added.

### Consequences

- `Entry*` and `Entry&` never leak out of `StorageEngine` again.
- Commands that used to mutate in place now build a new `Entry` and `Set` it. More code,  but less bugs.
- `Set` allocates a `shared_ptr` per write.
- `Entry` must stay immutable after construction.

---

## Response formatter is not a singleton

**Status:** accepted

### Decision**

`IResponse` is passed to commands through `Context`. No global
`std::cout` access inside commands.

### Why

- TCP frontend with many connections. Each connection needs its own output buffer.
- Easy testing: pass a fake formatter that records strings.

### Consequences

- `Context` holds `IResponse& out`.
- Commands never touch `std::cout` directly.

---

## Commands and `Context` are stateless between calls

### Decision

A command object has no mutable member state. Everything per call goes
through `Context`, which is created fresh for each input line and
deconstructed after that.

### Why

- MT. Stateless commands can be reused by many threads safely.

### Consequences

- No member variables in command classes (compile-time constants are fine).

