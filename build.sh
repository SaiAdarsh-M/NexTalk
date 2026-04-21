#!/bin/bash

echo "Checking crow files..."
ls Crow/include/crow

g++ server.cpp src/utils/Database.cpp -o server -std=c++17 -lpthread -I./Crow/include