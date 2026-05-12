cmake_minimum_required(VERSION 3.10)
project(SennaC)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

find_package(FLEX QUIET)
if(NOT FLEX_FOUND)
    message(WARNING "Flex not found. Attempting to install via apt-get...")
    execute_process(
        COMMAND sudo apt-get install -y flex
        RESULT_VARIABLE FLEX_INSTALL_RESULT
    )
    if(NOT FLEX_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to install Flex. Please install it manually.")
    endif()
    find_package(FLEX REQUIRED)
endif()


find_package(BISON QUIET)
if(NOT BISON_FOUND)
    message(WARNING "Bison not found. Attempting to install via apt-get...")
    execute_process(
        COMMAND sudo apt-get install -y bison
        RESULT_VARIABLE BISON_INSTALL_RESULT
    )
    if(NOT BISON_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to install Bison. Please install it manually.")
    endif()
    find_package(BISON REQUIRED)
endif()

add_subdirectory(src)
