CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread

all: BGRS_Client
	g++ -o bin/BGRS_Client bin/BGRS_ConnectionHandler.o bin/BGRS_Client.o $(LDFLAGS)

BGRS_Client: bin/BGRS_ConnectionHandler.o bin/BGRS_Client.o
	
bin/BGRS_ConnectionHandler.o: src/BGRS_ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/BGRS_ConnectionHandler.o src/BGRS_ConnectionHandler.cpp

bin/BGRS_Client.o: src/BGRS_Client.cpp
	g++ $(CFLAGS) -o bin/BGRS_Client.o src/BGRS_Client.cpp
	
.PHONY: clean
clean:
	rm -f bin/*
