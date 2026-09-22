# valkey-lite

A small in-memory key-value store written in C++23, implementing a
subset of the [Valkey](https://valkey.io/) / Redis command set.

> Status: **work in progress**. Storage core, comand dispatcher in progress. See [ROADMAP](docs/Roadmap.md) for future plans.

## What it is

- **In-memory, embeddable.** Single process, no external dependencies.
- **Valkey-compatible subset.** String, List, Set and Geospatial commands,
  plus a few generic ones. See the full list below.
- **C++23.** Uses `std::variant`, `std::visit`, Concepts, `std::expected`,
  `std::shared_mutex` (planned), and other modern facilities.
- **Planned:** multi-threaded execution with a TCP/RESP frontend.
  See [ROADMAP](docs/Roadmap.md).

## ## Requirements

- C++23-capable compiler (GCC 13+, Clang 17+, MSVC 19.36+).
- CMake 3.20+.
- (Optional) `-fsanitize=thread` support for concurrency testing.

## Run

Interactive REPL (commands from stdin, results to stdout):

    ./build/valkey-lite

With a memory limit (suffixes `b`, `kb`, `mb`, `gb`):

    ./build/valkey-lite --maxmemory 64mb

Exit with `EXIT` or `Ctrl-D`.

## Example session

    > SET greeting "hello"
    OK
    > APPEND greeting ", world"
    (integer) 12
    > GET greeting
    "hello, world"
    > EXPIRE greeting 60
    (integer) 1
    > TTL greeting
    (integer) 60

    > RPUSH queue a b c
    (integer) 3
    > LRANGE queue 0 -1
    1) "a"
    2) "b"
    3) "c"
    > LPOP queue
    "a"

    > SADD tags cpp redis
    (integer) 2
    > SMEMBERS tags
    1) "cpp"
    2) "redis"

    > GEOADD cities 30.31 59.94 "spb"
    (integer) 1
    > GEODIST cities spb spb
    "0.0000"

## Supported commands

Legend: `[x]` implemented · `[ ]` planned · `[-]` out of scope

### String

- [ ] `SET`, `GET`, `STRLEN`, `APPEND`, `EXPIRE`, `TTL`

### List

- [ ] `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LLEN`, `LRANGE`, `LINDEX`, `LSET`, `LINSERT`

### Set

- [ ] `SADD`, `SREM`, `SISMEMBER`, `SMEMBERS`, `SCARD`, `SUNION`, `SINTER`, `SDIFF`, `SMOVE`

### Geo

- [ ] `GEOADD`, `GEOPOS`, `GEODIST`, `GEOSEARCH`, `GEOSEARCHSTORE`

### Generic

- [ ] `TYPE`, `DEL`, `EXISTS`, `KEYS`, `FLUSHDB`, `CONFIG SET/GET`, `DBSIZE`, `MEMORY USAGE`

### Control

- [ ] `EXIT` / EOF

> Update the checkboxes as commands land. Do not let this list drift
> from reality — a stale status is worse than no status.

## Design

See [ARCHITECTURE](docs/ARCHITECTURE.md) for details.

## Roadmap

See [ROADMAP](docs/ROADMAP.md). Short version:

1. **Core.** Single-threaded storage + REPL. *(current)*
2. **Multi-threaded core.** `std::shared_mutex`, `shared_ptr<const Entry>`,
   TSan-clean stress tests.
3. **Network frontend.** TCP + RESP, thread-per-connection.
4. **Polish.** Benchmarks, Docker image, CI, documentation.

## Documentation

- [ARCHITECTURE](docs/ARCHITECTURE.md) — layers, responsibilities, data flow.
- [DECISIONS](docs/DECISIONS.md) — design decisions (ADR-lite).
- [CONTRACTS](docs/CONTRACTS.md) — class invariants and contracts.
- [ROADMAP](docs/ROADMAP.md) — plan and progress.

## License

SMTH open
