cmake_minimum_required(VERSION 3.10)
project(Client)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_HEADERS
        include/connectionHandler.h
        include/BGRS_ConnectionHandler.h
        )
set(PROJECT_SOURCES
        src/connectionHandler.cpp
        src/BGRS_ConnectionHandler.cpp
        src/echoClient.cpp
        src/BGRS_Client.cpp
        )

add_executable(Client ${PROJECT_SOURCES} ${PROJECT_HEADERS})
