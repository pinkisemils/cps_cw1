CXX=g++
t=time
SOURCES = $(wildcard *.cpp)
EXEC = $(SOURCES:.cpp=.out)
SLOW_EXEC = $(SOURCES:.cpp=slow.out)
TIME_OUT = $(EXEC:.out=.timed)	
PERF_PROFILES = $(EXEC:.out=.perf) $(SLOW_EXEC:.out=.perf)
FOLDED_STACKS = $(PERF_PROFILES:.perf=.folded)
FLAME_GRAPHS = $(FOLDED_STACKS:.folded=.svg) 
SC = stackcollapse-perf
FG = flamegraph


CXXFLAGS= -O3 -g -fopenmp -lpthread -std=c++14 -fno-omit-frame-pointer
SLOW_CXXFLAGS= -g -fopenmp -lpthread -std=c++14 -fno-omit-frame-pointer

%.out: %.cpp
	$(CXX) $< $(CXXFLAGS) -o $@

%.slow.out: %.cpp
	$(CXX) $< $(SLOW_CXXFLAGS) -o $@

build: $(EXEC)

slow: $(SLOW_EXEC)

all: build time



time: $(TIME_OUT)
%.timed: %.out
	 $(t) ./$< > $@

clean:
	rm -f *.out *.folded *.svg *.p1

clean_perf:
	rm -f *.perf

perf: $(PERF_PROFILES)

%.p1: %.out
	perf record -g --call-graph dwarf --output $@ ./$<

%.perf: %.p1
	perf script -i $< > $@

%.folded: %.perf
	$(SC) $< > $@

%.svg: %.folded
	$(FG) $< > $@

flamegraphs: $(FLAME_GRAPHS)

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
