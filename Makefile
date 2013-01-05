# copyright 2013 Jens Schwarzer (schwarzer@schwarzer.dk)

# if 'CL/cl.hpp' is used then add -DCL_USE_DEPRECATED_OPENCL_1_1_APIS until further
opencl-info: opencl-info.cpp
	g++ -Wall -std=c++11 -O3 -DCL_USE_DEPRECATED_OPENCL_1_1_APIS opencl-info.cpp -oopencl-info -lOpenCL

clean:
	rm opencl-info
