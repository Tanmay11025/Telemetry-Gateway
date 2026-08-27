FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN cmake -B build && cmake --build build

CMD ["./build/message_gateway"]
