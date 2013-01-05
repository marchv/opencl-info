# copyright 2013 Jens Schwarzer (schwarzer@schwarzer.dk)

opencl-info: opencl-info.cpp
	g++ -Wall -std=c++11 -O3 opencl-info.cpp -oopencl-info -lOpenCL

clean:
	rm opencl-info
