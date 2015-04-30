cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(ynet CXX)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")
endif()

include_directories(include)

add_library(ynet
	src/address.cpp
	src/backend.cpp
	src/client.cpp
	src/local.cpp
	src/main.cpp
	src/server.cpp
	src/socket.cpp
	src/tcp.cpp
	)

link_libraries(ynet)

add_executable(ynet-test
	main.cpp
	)

add_executable(ynet-benchmark
	benchmark/benchmark.cpp
	benchmark/connect_disconnect.cpp
	benchmark/exchange.cpp
	benchmark/receive.cpp
	benchmark/send.cpp
	benchmark/main.cpp
	)

set_property(TARGET ynet ynet-benchmark ynet-test PROPERTY CXX_STANDARD 14)
