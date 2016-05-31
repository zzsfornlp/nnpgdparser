#!/usr/bin/bash

g++ -O3 -Wall -I"$MKLROOT/include" src/*/*.cpp src/main.cpp -DBLAS_INTEL_MKL -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread -lboost_regex -o nnpgdp
