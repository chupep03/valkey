# Roadmap

Living checklist. Update as things land. Do not let it drift.

## Stage 1 — Core REPL (current)

Goal: a working single-threaded REPL, all commands implemented, basic tests.

- [x] `Tokenizer`, `ResponseFormatter`, `Args`, `Repl`
- [x] `MemoryManager`, `StorageEngine`, `Entry`, `ValueVariant`
- [x] `ICommand`, `Context`, `CommandReg`, `RegisterAll`
- [x] String commands (6)
- [x] List commands (9)
- [x] Set commands (9)
- [x] Generic commands (8)
- [x] Geo commands (6)
- [x] Unit tests for `MemoryManager`, `StorageEngine`, `Entry`
- [x] Unit tests for each command group
- [x] Manual smoke test of all commands
- [x] Update README command checklist
- [x] CI/CD

## Stage 2 — Multi-threaded core

Goal: concurrent access to a single `StorageEngine` without races.

- [ ] `std::shared_mutex` around `storage_` (readers vs writers)
- [ ] `std::atomic<size_t>` for `actual_usage_` in `MemoryManager`
- [ ] Atomic `Resize` (CAS loop or lock)
- [ ] TSan-clean stress test (N threads, random commands)
- [ ] Invariant check after stress: usage == Σ TotalSizeFor

## Stage 3 — Network frontend

Goal: TCP server with a RESP protocol.

- [ ] RESP protocol parser and writer
- [ ] Thread-per-connection server (simple)
- [ ] `ResponseFormatter` per connection
- [ ] `redis-cli` / `valkey-cli` can connect and run commands
- [ ] Graceful shutdown on SIGINT

## Stage 4 — Polish

- [ ] CMake: build tests, wire CTest
- [ ] CI (GitHub Actions): build + tests + TSan
- [ ] Dockerfile (multi-stage)
- [ ] Benchmark (100k SET/GET)
- [ ] Screenshot in README
- [ ] `LICENSE`, `.gitignore`
