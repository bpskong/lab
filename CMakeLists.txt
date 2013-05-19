project(bpslab)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS
	"${CMAKE_C_FLAGS} -std=c++0x"
)
set(SRC_LIST
	main.cpp
)
include_directories(${CMAKE_SOURCE_DIR})
add_executable(${PROJECT_NAME} ${SRC_LIST})
