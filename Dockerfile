FROM ubuntu:22.04

# Avoid prompts during install
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libasio-dev \
    libssl-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy all project files
COPY . .

# Compile
g++ server.cpp src/utils/Database.cpp -o server -std=c++17 -lpthread -lsqlite3 -I./Crow/include

# Expose port (Render uses dynamic PORT)
EXPOSE 10000

# Start server
CMD ["./server"]