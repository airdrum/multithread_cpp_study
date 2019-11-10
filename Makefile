all: main

main: main.cpp 
	g++ -o main main.cpp -pthread -std=c++11

