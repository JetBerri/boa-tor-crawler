# --- Build stage ---
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    libcurl4-openssl-dev \
    libyaml-dev \
    libpq-dev \
    make \
    ca-certificates \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN make clean && make

# Runtime stage 
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpq5 \
    ca-certificates \
    libcurl4 \
    libyaml-0-2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/tor-crawler /app/tor-crawler
COPY --from=builder /app/config /app/config
COPY --from=builder /app/logs /app/logs

# ENV variables
ENV TOR_PROXY=socks5h://tor-proxy:9050
ENV DB_HOST=tor-postgres
ENV DB_USER=crawler
ENV DB_PASS=crawlerpass
ENV DB_NAME=torcrawl

# Launch Crawler
ENTRYPOINT ["/app/tor-crawler"]
