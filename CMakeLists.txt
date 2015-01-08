project(YNET CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=gnu++0x -Wall")
add_executable(ynet
	main.cpp
	tcp_backend.cpp
	tcp_client.cpp
	tcp_server.cpp
	ynet.cpp)
