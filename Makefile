all: laby pgmtoobj

laby: laby.cpp
	g++ -std=c++0x -O2 -Wall -lm -g -I.. -o laby laby.cpp

pgmtoobj:pgmtoobj.cpp
	g++ -Wall -g -O2 -I.. -o pgmtoobj pgmtoobj.cpp
