#!/bin/bash
g++ -std=c++17 -I line-reader -I lazy-write md-patcher.cpp lazy-write/lazy-write.cpp line-reader/line-reader.cpp -lstdc++fs -o md-patcher
