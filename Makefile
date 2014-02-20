# copyright 2013 Jens Schwarzer (schwarzer@schwarzer.dk)

ifeq ($(shell uname), Darwin) # Apple
    LIBOPENCL=-framework OpenCL
else       # Linux
    LIBOPENCL=-L$(OPENCLLIB) -lOpenCL
endif



opencl-info: opencl-info.cpp
	g++ -Wall -std=c++11 -O3 opencl-info.cpp -oopencl-info $(LIBOPENCL)

clean:
	rm opencl-info
