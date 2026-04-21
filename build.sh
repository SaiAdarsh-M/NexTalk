#!/bin/bash

# download crow
git clone https://github.com/CrowCpp/Crow.git

# compile
g++ server.cpp src/utils/Database.cpp -o server -std=c++17 -lpthread -ICrow/include