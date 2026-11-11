FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR  /app

COPY CMakeLists.txt ./
COPY lib/ ./lib/
COPY bin/ ./bin/

RUN cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVALKEY_BUILD_TESTS=OFF
RUN cmake --build build --parallel

# multy stage

FROM ubuntu:24.04
WORKDIR /app
COPY --from=builder /app/build/valkey /app/valkey
CMD ["./valkey", "--maxmemory", "64mb"]