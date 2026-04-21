#!/bin/bash
g++ server.cpp src/utils/Database.cpp -o server -std=c++17 -lpthread -ICrow/include