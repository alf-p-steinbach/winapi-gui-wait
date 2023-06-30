# Generating msbuild / Visual Studio project for Visual C++:
# cmake ..\..

# Generating makefile for MinGW g++:
# cmake -G"MinGW Makefiles" ..\.. -D CMAKE_C_COMPILER=gcc -D CMAKE_CXX_COMPILER=g++

cmake_minimum_required( VERSION 3.1 )

project( gui-wait-example LANGUAGES CXX )
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( SOURCES source/main.cpp source/resources.rc )

#the file(GLOB...) allows for wildcard additions:
#file( GLOB SOURCES "source/*.cpp" )

add_executable( gui-wait-example ${SOURCES})
target_compile_features( gui-wait-example PUBLIC cxx_std_17 )

include_directories( source )
if( MSVC )
    # warning level 4
    add_compile_options( /W4 )
else()
    # additional warnings, g++ and clang
    add_compile_options( -Wall -Wextra -Wpedantic-errors)
endif()

target_link_libraries( gui-wait-example comctl32 )
if( MSVC )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no /entry:mainCRTStartup" )
endif()
