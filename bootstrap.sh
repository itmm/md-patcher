#!/bin/bash
g++ -std=c++17 \
  -I assert-problems/include \
  -I line-reader/include \
  -I lazy-write/include \
  md-patcher.cpp \
  assert-problems/solid/require.cpp \
  lazy-write/lazy-write.cpp \
  line-reader/line-reader.cpp \
  -o mdp