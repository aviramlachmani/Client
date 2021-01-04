CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread

all: BGRSclient
	g++ -o bin/BGRSclient bin/BGRS_ConnectionHandler.o bin/Task.o bin/BGRSclient.o $(LDFLAGS)

BGRSclient: bin/BGRS_ConnectionHandler.o bin/Task.o bin/BGRSclient.o
	
bin/BGRS_ConnectionHandler.o: src/BGRS_ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/BGRS_ConnectionHandler.o src/BGRS_ConnectionHandler.cpp

bin/Task.o: src/Task.cpp
	g++ $(CFLAGS) -o bin/Task.o src/Task.cpp

bin/BGRSclient.o: src/BGRSclient.cpp
	g++ $(CFLAGS) -o bin/BGRSclient.o src/BGRSclient.cpp


	
.PHONY: clean
clean:
	rm -f bin/*
