project(bpslab)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS
	"${CMAKE_C_FLAGS} -std=c++0x"
)
set(SRC_LIST
	global.h
	atomic.h
	ref.h
	ref.cpp
	str.h
	str.cpp
	io.h
	io.cpp
	zip.h
	zip.cpp
	main.cpp
)
link_libraries(/usr/lib/libz.a)
include_directories(${CMAKE_SOURCE_DIR})
add_executable(${PROJECT_NAME} ${SRC_LIST})
