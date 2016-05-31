#!/usr/bin/bash

g++ -O3 -Wall src/*/*.cpp src/main.cpp -DBLAS_ATLAS -lblas -lboost_regex -o nnpgdp
