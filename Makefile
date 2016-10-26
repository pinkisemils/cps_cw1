CXX=g++
t=time
all: build time

build:	threaded serial openmp

threaded:  threaded.cpp
	$(CXX) threaded.cpp -O3 -g -fopenmp -o threaded 

serial: serial.cpp
	$(CXX) serial.cpp -O3 -o serial

openmp: openmp.cpp
	$(CXX) openmp.cpp -O3 -g -fopenmp -lpthread -o openmp

time:	threaded serial openmp
	$(t) ./threaded > /dev/null
	$(t) ./serial > /dev/null
	$(t) ./openmp > /dev/null

clean:
	rm threaded serial openmp

perf: threaded serial openmp
	perf record --output openmp.perf ./openmp
	perf record --output serial.perf ./serial
	perf record --output threaded.perf ./threaded
