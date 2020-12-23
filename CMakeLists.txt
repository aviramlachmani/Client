cmake_minimum_required(VERSION 3.10)
project(Client)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_HEADERS
        include/connectionHandler.h
        )
set(PROJECT_SOURCES
        src/connectionHandler.cpp
        src/echoClient.cpp
        )

add_executable(Client ${PROJECT_SOURCES} ${PROJECT_HEADERS})
