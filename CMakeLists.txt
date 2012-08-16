cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/modules/")

PROJECT(dcompile)

INCLUDE (CheckIncludeFiles)
include (FindPkgConfig)

# build type
set(CMAKE_BUILD_TYPE Debug)

# compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")

# compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-W -Wall -O3")
set(CMAKE_C_FLAGS_RELEASE "-W -Wall -O3")

#if you don't want the full compiler output, remove the following line
set(CMAKE_VERBOSE_MAKEFILE ON)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.42.0 COMPONENTS thread filesystem system REQUIRED )
if (NOT Boost_FOUND)
    message(SEND_ERROR "Boost not found - skipping building tests")
endif (NOT Boost_FOUND)

find_package(LLVM REQUIRED )
if (NOT LLVM_FOUND)
    message(SEND_ERROR "LLVM not found - skipping building tests")
endif (NOT LLVM_FOUND)

find_package(Clang REQUIRED )
if (NOT CLANG_FOUND)
    message(SEND_ERROR "Clang not found - skipping building tests")
endif (NOT CLANG_FOUND)

find_package(LibDL REQUIRED )
if (NOT LIBDL_FOUND)
    message(SEND_ERROR "libdl not found - skipping building tests")
endif (NOT LIBDL_FOUND)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include ${Boost_INCLUDE_DIRS} ${LLVM_INCLUDE_DIR} ${CLANG_INCLUDE_DIR} ${LIBDL_INCLUDE_DIR} )
LINK_DIRECTORIES( ${LLVM_LIB_DIR} )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_COMPILE_FLAGS}" )
SET( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} ${LLVM_LIBS} -Wl,-E" )
subdirs( include src )
