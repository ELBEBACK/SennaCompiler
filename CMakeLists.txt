cmake_minimum_required(VERSION 3.10)
project(SennaC)

set(CMAKE_CXX_STANDARD 17)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graph_dump

)

flex_target(MyLexer src/frontend/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
bison_target(MyParser src/frontend/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
add_flex_bison_dependency(MyLexer MyParser)

add_executable(senna
    src/main.cpp
    src/graph_dump/dot_print.cpp
    ${FLEX_MyLexer_OUTPUTS}
    ${BISON_MyParser_OUTPUTS}
)
