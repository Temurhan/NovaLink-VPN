FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libcurl4-openssl-dev \
    libsqlite3-dev \
    wget \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir -p core/include && \
    wget -O core/include/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h && \
    wget -O core/include/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp

RUN mkdir -p build && cd build && cmake .. && make novalink_server

FROM ubuntu:22.04

WORKDIR /app

RUN apt-get update && apt-get install -y \
    libssl3 \
    libcurl4 \
    libsqlite3-0 \
    iptables \
    iproute2 \
    openssl \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/bin/novalink_server .e

RUN chmod +x novalink_server

EXPOSE 443 8080

CMD ["./novalink_server"]