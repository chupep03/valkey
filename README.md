# valkey

[![CI](https://github.com/chupep03/valkey/actions/workflows/ci.yml/badge.svg)](https://github.com/chupep03/valkey/actions/workflows/ci.yml)

In-memory **key-value** store, with a subset of the [Valkey](https://valkey.io/) / Redis commands.

## What it is

- **In-memory** Single process, no external dependencies.
- **Valkey-compatible subset.** String, List, Set, Geo, and Generic
  commands. See the list below.
- **C++23.** Uses `std::variant`, `std::visit`, the Overload pattern, `std::shared_ptr`.
- **Thread-safe.** One `std::shared_mutex` inside `StorageEngine`;
  readers and writers run in parallel. Tested under TSan.
- **Planned:** TCP/RESP frontend. See [docs/ROADMAP.md](docs/ROADMAP.md).

## Requirements

- `C++23` compiler
- `CMake 3.20+`

## Build

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j

## Run

    ./build/valkey
    ./build/valkey --maxmemory 64mb

Exit with `EXIT` (case-insensitive) or `Ctrl-D`.

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

    > GEOADD cities 30.31 59.94 spb
    (integer) 1
    > GEODIST cities spb spb
    "0.0000"
    > exit

## Tests

    cmake -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build -j
    ctest --test-dir build --output-on-failure

Filter:

    ./build/valkey_tests --gtest_filter='SetCommands.*'

## Sanitizers

ThreadSanitizer (race detection for the MT core):

    cmake -B build-tsan -DVALKEY_TSAN=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build build-tsan -j
    ./build-tsan/valkey_tests --gtest_filter='StressMT.*'

## Supported commands

**String**:
`SET`, `GET`, `STRLEN`, `APPEND`, `EXPIRE`, `TTL`

**List**:
`LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LLEN`, `LRANGE`, `LINDEX`, `LSET`, `LINSERT`

**Set**:
`SADD`, `SREM`, `SISMEMBER`, `SMEMBERS`, `SCARD`, `SUNION`, `SINTER`, `SDIFF`, `SMOVE`

**Geo**:
`GEOADD`, `GEOPOS`, `GEODIST`, `GEOSEARCH`, `GEOSEARCHSTORE`

**Generic**:
`TYPE`, `DEL`, `EXISTS`, `KEYS`, `FLUSHDB`, `DBSIZE`, `MEMORY USAGE`,
`CONFIG SET/GET maxmemory`

**Control**
`EXIT`

## Project layout

    bin/main.cpp                  — thin entry point
    lib/
      storage/                    — data layer (Entry, StorageEngine, MemoryManager)
      commands/                   — command layer (ICommand, Context, CommandReg)
        string/ list/ set/ geo/ generic/
      parser/                     — Tokenizer
      application/                — Args, Repl, ResponseFormatter
    docs/                         — ARCHITECTURE, DECISIONS, ROADMAP, CONTRACTS
    tests/                        — gtests

## Documentation

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — layers, responsibilities, data flow.
- [docs/DECISIONS.md](docs/DECISIONS.md) — design decisions.
- [docs/ROADMAP.md](docs/ROADMAP.md) — plan and progress.
- [docs/CONTRACTS.md](docs/CONTRACTS.md) — class invariants.

## License

MIT