cmake_minimum_required(VERSION 3.0)
project(ynet CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=gnu++11 -Wall")
add_executable(ynet
	main.cpp
	tcp_backend.cpp
	tcp_client.cpp
	tcp_connection.cpp
	tcp_server.cpp
	ynet.cpp)
