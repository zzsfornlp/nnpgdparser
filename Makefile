## the makefile for nn-graph-dparser(nnpgdp)
#to compile the parser, we need boost_regex and blas library

CPP=g++
CC=gcc
CFLAGS=-O3 
LFLAGS=-O3
LD=g++

#!!!specify the blas lib location and the blas lib, these may not be the same in different machines
BLAS_LIBS_LOCATION=-L/usr/lib/atlas-blas
BLAS_LIBS=-lblas  
#BLAS_LIBS=-lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread

###choose the blas implementation - default atlas
BLAS_DEFINE=-DBLAS_ATLAS
#BLAS_DEFINE=-DBLAS_INTEL_MKL

SRCS=$(wildcard src/*/*.cpp) src/main.cpp
#no source files with the same name
OBJS=$(patsubst %.cpp,obj/%.o,$(notdir $(SRCS)))

nnpgdp: obj/depends $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o nnpgdp $(BLAS_LIBS_LOCATION) -lboost_regex $(BLAS_LIBS)

obj/depends:
	$(CPP) $(CFLAGS) $(BLAS_DEFINE) -MM $(SRCS) > $@
    
obj/%.o: src/%.cpp
	$(CPP) $(CFLAGS) $(BLAS_DEFINE) -c $< -o $@
obj/%.o: src/algorithms/%.cpp
	$(CPP) $(CFLAGS) $(BLAS_DEFINE) -c $< -o $@
obj/%.o: src/csnn/%.cpp
	$(CPP) $(CFLAGS) $(BLAS_DEFINE) -c $< -o $@
obj/%.o: src/parts/%.cpp
	$(CPP) $(CFLAGS) $(BLAS_DEFINE) -c $< -o $@
obj/%.o: src/graph-based/%.cpp
	$(CPP) $(CFLAGS) $(BLAS_DEFINE) -c $< -o $@
obj/%.o: src/tools/%.cpp
	$(CPP) $(CFLAGS) $(BLAS_DEFINE) -c $< -o $@

include obj/depends

.PHONY: clean
clean:
	rm -f obj/depends obj/*.o nnpgdp