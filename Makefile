CXX=g++
all: fastm slowm serial

fastm:  main.cpp
	$(CXX) main.cpp -O3 -g -fopenmp -o fastm 
slowm:  main.cpp
	$(CXX) main.cpp -O0 -g -fopenmp -o slowm 

serial: original.cpp
	$(CXX) original.cpp -O3 -o serial

clean:
	rm fastm slowm
