CXX=g++
all: fastm slowm

fastm:  main.cpp
	$(CXX) main.cpp -O3 -g -o fastm 
slowm:  main.cpp
	$(CXX) main.cpp -O0 -g -o slowm 

clean:
	rm fastm slowm
