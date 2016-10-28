CXX=g++
t=time
SOURCES = $(wildcard *.cpp)
EXEC = $(SOURCES:.cpp=.out)
SLOW_EXEC = $(SOURCES:.cpp=slow.out)


CXXFLAGS= -O3 -g -fopenmp -lpthread -std=c++14
SLOW_CXXFLAGS= -g -fopenmp -lpthread -std=c++14 

%.out: %.cpp
	$(CXX) $< $(CXXFLAGS) -o $@

%slow.out: %.cpp
	$(CXX) $< $(SLOW_CXXFLAGS) -o $@

build: $(EXEC)

slow: $(SLOW_EXEC)

all: build time


time: $(EXEC)
	 $(foreach e,$(EXEC), echo $(e) && $(t) ./$(e) ;)

clean:
	rm -f *.out

perf:
	perf record -g --output openmp.perf ./openmp
	perf script -i openmp.perf | c++filt | gprof2dot -f perf | dot -Tpng -o openmp-perf.png
	perf record -g --output serial.perf ./serial
	perf script -i serial.perf | c++filt | gprof2dot -f perf | dot -Tpng -o serial-perf.png
	perf record -g --output threaded.perf ./threaded
	perf script -i threaded.perf | c++filt | gprof2dot -f perf | dot -Tpng -o threaded-perf.png
	perf record -g --output futures.perf ./futures
	perf script -i futures.perf | c++filt | gprof2dot -f perf | dot -Tpng -o futures-perf.png

callgrind:
	valgrind --tool=callgrind --callgrind-out-file=openmp.cg ./openmp
	gprof2dot -f callgrind <openmp.cg | dot -Tpng -o openmp-cg.png
	valgrind --tool=callgrind --callgrind-out-file=serial.cg ./serial
	gprof2dot -f callgrind <serial.cg | dot -Tpng -o serial-cg.png
	valgrind --tool=callgrind --callgrind-out-file=threaded.cg ./threaded
	gprof2dot -f callgrind <threaded.cg | dot -Tpng -o threaded-cg.png
	valgrind --tool=callgrind --callgrind-out-file=futures.cg ./futures
	gprof2dot -f callgrind <futures.cg | dot -Tpng -o futures-cg.png
.PHONY: build
