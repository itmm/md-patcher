#!/bin/bash
c++ -std=c++20 \
  -I solid-require/include -I line-reader/include -I lazy-write/include \
  md-patcher.cpp solid-require/solid/require.cpp \
  lazy-write/lazy-write.cpp line-reader/line-reader.cpp \
  -o mdp
