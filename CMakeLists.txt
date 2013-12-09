#--------------------------------------
# CMake configuration
#--------------------------------------
cmake_minimum_required (VERSION 2.8)


#--------------------------------------
# Build configuration
#--------------------------------------
project (NLang)
set (NLANG_VERSION_MAJOR 0)
set (NLANG_VERSION_MINOR 0)
set (NLANG_VERSION_PATCH 0)

# Build type (Release, Debug)
set (CMAKE_BUILD_TYPE Release)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file ("${PROJECT_SOURCE_DIR}/config.h.in"
                "${PROJECT_BINARY_DIR}/config.h")

# C++11 for gcc and clang
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -pthread")

# Warnings
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-function")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function")

# Warnings
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Define "DEBUG" in debug builds
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")


#--------------------------------------
# Boost
#--------------------------------------

set (Boost_MULTITHREADED TRUE)
set (Boost_USE_STATIC_LIBS FALSE)

find_package (Boost 1.48 COMPONENTS program_options unit_test_framework REQUIRED)

include_directories (${Boost_INCLUDE_DIRS})
link_directories (${Boost_LIBRARY_DIRS})



#--------------------------------------
# Project include directories
#--------------------------------------
include_directories ("${PROJECT_BINARY_DIR}",
					 "${PROJECT_SOURCE_DIR}",
					 "${PROJECT_SOURCE_DIR}/test"
					 "${PROJECT_SOURCE_DIR}/lexer",
					)


#--------------------------------------
# Build
#--------------------------------------
add_subdirectory(lexer)



#add_library(config
#	config)
#add_library(global
#	global)

# Main library
set (NLANG_LIB
	lexer
	)

# Main executable
add_executable(nlang
	main)
target_link_libraries(nlang
	${NLANG_LIB}
	${Boost_LIBRARIES}
	)

# Unit-test executable
file(GLOB_RECURSE TEST_FILES *_test.cpp) # Find all tests

add_executable(unit_tests
	"test/test_main" ${TEST_FILES})
target_link_libraries(unit_tests
	${NLANG_LIB}
	${Boost_LIBRARIES}
	)
