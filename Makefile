CXX=g++
t=time
SOURCES = $(wildcard *.cpp)
EXEC = $(SOURCES:.cpp=.out)
SLOW_EXEC = $(SOURCES:.cpp=slow.out)
TIME_OUT = $(EXEC:.out=.timed)	
PERF_PROFILES = $(EXEC:.out=.perf) $(SLOW_EXEC:.out=.perf)
FOLDED_STACKS = $(PERF_PROFILES:.perf=.folded)
FLAME_GRAPHS = $(FOLDED_STACKS:.folded=.svg) 
CALLGRIND_PROF = $(EXEC:.out=.cg) $(SLOW_EXEC:.out=.cg)
CALLGRIND_PROF_PNG = $(CALLGRIND_PROF:.cg=.cg.png)
SC = stackcollapse-perf
FG = flamegraph


CXXFLAGS= -O3 -g -fopenmp -lpthread -std=c++11 -fno-omit-frame-pointer
SLOW_CXXFLAGS= -g -fopenmp -lpthread -std=c++11 -fno-omit-frame-pointer

%.out: %.cpp
	$(CXX) $< $(CXXFLAGS) -o $@

%slow.out: %.cpp
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

callgrind: $CALLGRIND_PROF

%.cg: %.out
	valgrind --tool=callgrind --callgrind-out-file=$@ ./$<

%.cg.png: %.cg
	gprof2dot -f callgrind <$< | dot -Tpng -o $@

.PHONY: build
