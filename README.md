## nnpgdparser: Probabilistic Graph-based Dependency Parser with Neural Netwrok ##
------------------------------------------------------

### Intro ###

This repo contains the implementation for a probabilistic graph-based dependency parser with neural network. The parser is written in c++, and use `feed-forward + convolutional` neural network models for high-order graph-based dependency parsing.

This could be regarded as an update version of [nngdparser](https://github.com/zzsfornlp/nngdparser), with new neural-network implementation, and better training methods.

### How to compile ###

Change to the top-layer directory of the project, and Run "make" or directly run "bash compile_mkl.sh" or "bash compile_blas.sh" for one-time compiling.

The "nnpgdp" is the runnable file for the parser.

This is the environment where we compile it, if you are interested in compiling in other environments, please figure out the library dependents.

	Platform: Linux os
	Compiler: g++, gcc
	Libraries: Boost C++ libraries, Blas (atlas or mkl).

For more informations of compiling and the libraries, please check out the makefile or the compile script.

### How to run ###

For the training and testing part, please check out the `doc/Usage.txt` file for details.

### More Documentation ###

Please check the `doc` for more details, starting with `doc/INDEX.txt` will be a good option.

### Related paper ###

This is the implementation of our paper in ACL-2016: [link](http://www.aclweb.org/anthology/P/P16/P16-1131.pdf).

	@InProceedings{zhang-zhao-qin:2016:P16-1,
	  author    = {Zhang, Zhisong and Zhao, Hai and Qin, Lianhui},
	  title     = {Probabilistic Graph-based Dependency Parsing with Convolutional Neural Network},
	  booktitle = {Proceedings of the 54th Annual Meeting of the Association for Computational Linguistics (Volume 1: Long Papers)},
	  month     = {August},
	  year      = {2016},
	  address   = {Berlin, Germany},
	  publisher = {Association for Computational Linguistics},
	  pages     = {1382--1392},
	  url       = {http://www.aclweb.org/anthology/P16-1131}
	}
