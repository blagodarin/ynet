cmake_minimum_required(VERSION 3.0)
project(ynet CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=gnu++11 -Wall")
include_directories(include)

add_library(ynet
	src/address.cpp
	src/client.cpp
	src/connection.cpp
	src/server.cpp
	src/tcp.cpp
	src/ynet.cpp)

add_executable(ynet-test
	main.cpp)
target_link_libraries(ynet-test ynet)

add_executable(ynet-benchmark
	benchmark/benchmark.cpp
	benchmark/connect_disconnect.cpp
	benchmark/exchange.cpp
	benchmark/receive.cpp
	benchmark/send.cpp
	benchmark/main.cpp)
target_link_libraries(ynet-benchmark ynet)
