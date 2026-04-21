#!/bin/bash

echo "Installing dependencies..."
apt-get update
apt-get install -y libasio-dev

echo "Building project..."
g++ server.cpp src/utils/Database.cpp -o server -std=c++17 -lpthread -I./Crow/include