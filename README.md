# PointDetector

Detect points by line intersection(determined with small error):

- Accumulate intersection of three line with small distances
- Applying K-Means to this points

Build by CMake on Mac OS

Requirements:
1. xtensor:
	Debian: sudo apt-get install xtensor-dev
	Mac OS: brew install xtensor

2. xtensot-blas:
	From sources:
		https://github.com/xtensor-stack/xtensor-blas
		mkdir build
		cd build
		cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
		make install 
