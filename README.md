# valkey-lite

A small in-memory key-value store written in C++23, implementing a
subset of the [Valkey](https://valkey.io/) / Redis command set.

## What it is

- **In-memory** Single process, no external dependencies.
- **Valkey-compatible subset.** String, List, Set, Geo, and Generic
  commands. See the list below.
- **C++23.** Uses `std::variant`, `std::visit`, the Overload pattern,
  `std::from_chars`, `std::shared_ptr<const Entry>`
- **Planned:** multi-threaded core and a TCP/RESP frontend.
  See [docs/ROADMAP.md](docs/ROADMAP.md).

## Requirements

- C++23 compiler (`GCC 13+`, `Clang 17+`)
- `CMake 3.20+`

## Build

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j

## Run

    ./build/valkey
    ./build/valkey --maxmemory 64mb

Exit with `EXIT` command.

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

## Supported commands

**String**
`SET`, `GET`, `STRLEN`, `APPEND`, `EXPIRE`, `TTL`

**List**
`LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LLEN`, `LRANGE`, `LINDEX`, `LSET`, `LINSERT`

**Set**
`SADD`, `SREM`, `SISMEMBER`, `SMEMBERS`, `SCARD`, `SUNION`, `SINTER`, `SDIFF`, `SMOVE`

**Geo**
`GEOADD`, `GEOPOS`, `GEODIST`, `GEOSEARCH`, `GEOSEARCHSTORE`

**Generic**
`TYPE`, `DEL`, `EXISTS`, `KEYS`, `FLUSHDB`, `DBSIZE`, `MEMORY USAGE`,
`CONFIG SET/GET maxmemory`

**Control**
`EXIT`, EOF

## Project layout

    bin/main.cpp                  — thin entry point
    lib/
      storage/                    — data layer (Entry, StorageEngine, MemoryManager)
      commands/                   — command layer (ICommand, Context, CommandReg)
        string/ list/ set/ geo/ generic/
      parser/                     — Tokenizer
      application/                — Args, Repl, ResponseFormatter
    docs/                         — README, ARCHITECTURE, DECISIONS, ROADMAP, CONTRACTS
    tests/                        — gtests

## Documentation

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — layers, responsibilities, data flow.
- [docs/DECISIONS.md](docs/DECISIONS.md) — design decisions (ADR-lite).
- [docs/ROADMAP.md](docs/ROADMAP.md) — plan and progress.
- [docs/CONTRACTS.md](docs/CONTRACTS.md) — class invariants and contracts.

## License

MIT