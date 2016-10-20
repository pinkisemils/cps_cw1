CXX=g++
all: fastm slowm

fastm:  main.cpp
	$(CXX) main.cpp -O3 -fopenmp -g -o fastm 
slowm:  main.cpp
	$(CXX) main.cpp -O0 -fopenmp -g -o slowm 

clean:
	rm fastm slowm
