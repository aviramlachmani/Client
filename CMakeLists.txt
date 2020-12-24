cmake_minimum_required(VERSION 3.10)
project(Client)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_HEADERS
         include/BGRS_ConnectionHandler.h
        )
set(PROJECT_SOURCES
        src/BGRS_Client.cpp
        src/BGRS_ConnectionHandler.cpp
        )

add_executable(Client ${PROJECT_SOURCES} ${PROJECT_HEADERS})


