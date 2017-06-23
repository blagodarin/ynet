cmake_minimum_required(VERSION 3.1)

project(ynet CXX)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)

if(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-Wall)
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

target_link_libraries(ynet Threads::Threads)

link_libraries(ynet)

add_executable(ynet-benchmark
	benchmark/benchmark.cpp
	benchmark/connect_disconnect.cpp
	benchmark/exchange.cpp
	benchmark/receive.cpp
	benchmark/send.cpp
	benchmark/main.cpp
	)

add_executable(ynet-tcp
	examples/tcp.cpp
	)

add_executable(ynet-tests
	tests/common.cpp
	tests/local.cpp
	tests/tcp.cpp
	tests/utils.cpp
	)
target_link_libraries(ynet-tests GTest::GTest GTest::Main)

# Unfortunately CTest consumes the test output,
# and there is no way to retrieve it in its original form.
add_custom_target(check ynet-tests)
