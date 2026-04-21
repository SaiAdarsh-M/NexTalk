FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libasio-dev \
    libssl-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN g++ server.cpp $(find src -name "*.cpp") -o server -std=c++17 -lpthread -lsqlite3 -I./Crow/include

EXPOSE 18080

CMD ["./server"]