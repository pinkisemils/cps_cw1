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

perf:
	perf record -g --output openmp.perf ./openmp
	perf script -i openmp.perf | c++filt | gprof2dot -f perf | dot -Tpng -o openmp-perf.png
	perf record -g --output serial.perf ./serial
	perf script -i serial.perf | c++filt | gprof2dot -f perf | dot -Tpng -o serial-perf.png
	perf record -g --output threaded.perf ./threaded
	perf script -i threaded.perf | c++filt | gprof2dot -f perf | dot -Tpng -o threaded-perf.png

callgrind:
	valgrind --tool=callgrind --callgrind-out-file=openmp.cg ./openmp
	gprof2dot -f callgrind <openmp.cg | dot -Tpng -o openmp-cg.png
	valgrind --tool=callgrind --callgrind-out-file=serial.cg ./serial
	gprof2dot -f callgrind <serial.cg | dot -Tpng -o serial-cg.png
	valgrind --tool=callgrind --callgrind-out-file=threaded.cg ./threaded
	gprof2dot -f callgrind <threaded.cg | dot -Tpng -o threaded-cg.png

