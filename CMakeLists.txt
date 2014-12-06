#--------------------------------------
# CMake configuration
#--------------------------------------
cmake_minimum_required (VERSION 2.8)


#--------------------------------------
# Build configuration
#--------------------------------------
project (Rune)
set (RUNE_VERSION_MAJOR 0)
set (RUNE_VERSION_MINOR 0)
set (RUNE_VERSION_PATCH 0)

# Build type (Release, Debug)
set (CMAKE_BUILD_TYPE Debug)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file ("${PROJECT_SOURCE_DIR}/config.h.in"
                "${PROJECT_BINARY_DIR}/config.h")

# C++11 for gcc and clang
if (MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_ITERATOR_DEBUG_LEVEL=0")
else()
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

endif()


#--------------------------------------
# Project include directories
#--------------------------------------
include_directories ("${PROJECT_BINARY_DIR}"
					 "${PROJECT_SOURCE_DIR}"
					 "${PROJECT_SOURCE_DIR}/test"
					 "${PROJECT_SOURCE_DIR}/ast"
					 "${PROJECT_SOURCE_DIR}/lexer"
					 "${PROJECT_SOURCE_DIR}/parser"
					 "${PROJECT_SOURCE_DIR}/tokens"
                     "${PROJECT_SOURCE_DIR}/utils"
					)


#--------------------------------------
# Build
#--------------------------------------
add_subdirectory(ast)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(utils)



#add_library(config
#	config)
#add_library(global
#	global)

# Main library
set (RUNE_LIB
	lexer
	parser
	)

# Main executable
add_executable(rune
	main)
target_link_libraries(rune
	${RUNE_LIB}
	)

# Unit-test executable
file(GLOB_RECURSE TEST_FILES *_test.cpp) # Find all tests

add_executable(unit_tests
	"test/test_main" ${TEST_FILES})
target_link_libraries(unit_tests
	${RUNE_LIB}
	)
