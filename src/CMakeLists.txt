flex_target(MyLexer frontend/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
bison_target(MyParser frontend/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
add_flex_bison_dependency(MyLexer MyParser)

add_library(senna_compiler STATIC
    frontend/semantic.cpp
    midend/ir/ir.cpp
    midend/ir/ir_builder.cpp
    midend/ir/ir_verify.cpp
    midend/cfg/cfg_builder.cpp
    midend/dom/dom_tree.cpp
    midend/fdom/dom_front.cpp
    midend/ssa/phi_placement.cpp
    midend/ssa/mem2reg.cpp
    dumper/ir_print.cpp
    dumper/ast_print.cpp
    dumper/cfg_print.cpp
    dumper/doms_print.cpp
    ${FLEX_MyLexer_OUTPUTS}
    ${BISON_MyParser_OUTPUTS}
)

target_include_directories(senna_compiler
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include/cli
    ${CMAKE_SOURCE_DIR}/include/frontend
    ${CMAKE_SOURCE_DIR}/include/midend/ir
    ${CMAKE_SOURCE_DIR}/include/midend/cfg
    ${CMAKE_SOURCE_DIR}/include/midend/ssa
    ${CMAKE_SOURCE_DIR}/include/midend/dom
    ${CMAKE_SOURCE_DIR}/include/midend/fdom
    ${CMAKE_SOURCE_DIR}/include/dumper
    ${CMAKE_CURRENT_BINARY_DIR}
)
add_executable(senna main.cpp)
target_link_libraries(senna PRIVATE senna_compiler)

install(TARGETS senna senna_compiler
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
