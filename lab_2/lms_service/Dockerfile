FROM --platform=linux/amd64 ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS builder

WORKDIR /app

COPY configs/ configs/
COPY src/ src/
COPY tests/ tests/
COPY cmake/ cmake/
COPY CMakeLists.txt CMakePresets.json Makefile ./
COPY requirements.txt ./

RUN cmake --preset release -DUSERVER_FEATURE_TESTSUITE=OFF \
    && cmake --build build-release -j$(nproc) --target lms_service

FROM --platform=linux/amd64 ubuntu:24.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        curl \
        libssl3t64 \
        libcurl4t64 \
        libev4t64 \
        libnghttp2-14 \
        libc-ares2 \
        libboost-iostreams1.83.0 \
        libboost-filesystem1.83.0 \
        libboost-locale1.83.0 \
        libboost-program-options1.83.0 \
        libboost-regex1.83.0 \
        libboost-stacktrace1.83.0 \
        libcrypto++8t64 \
        libjemalloc2 \
        libre2-10 \
        libfmt9 \
        libcctz2 \
        libyaml-cpp0.8 \
        zlib1g \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build-release/lms_service /app/lms_service
COPY configs/ /app/configs/

EXPOSE 8080

CMD ["/app/lms_service", \
     "--config", "/app/configs/static_config.yaml", \
     "--config_vars", "/app/configs/config_vars.yaml"]