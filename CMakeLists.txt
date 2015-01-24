cmake_minimum_required(VERSION 3.0)
project(ynet CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=gnu++11 -Wall")
include_directories(include)
add_executable(ynet
	src/address.cpp
	src/client.cpp
	src/connection.cpp
	src/server.cpp
	src/tcp.cpp
	src/ynet.cpp
	main.cpp)
