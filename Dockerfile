FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libasio-dev \
    libssl-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Compile
RUN g++ server.cpp src/utils/Database.cpp -o server -std=c++17 -lpthread -lsqlite3 -I./Crow/include

# Expose port
EXPOSE 18080

# Run server
CMD ["./server"]