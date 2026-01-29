add_compile_options(-g -Wall -Wswitch -O0 --coverage)
add_link_options(--coverage)

set(HEADERS
    ../include/exp.h
    ../include/interp.h
    ../include/ir.h
    ../include/parser.h
    ../include/lexer.h
    ../include/semantics.h
    ../include/stmt.h
    ../include/structs.h
    ../include/utils.h
)

set(SOURCES
    exp.c   
    ir.c
    interp.c 
    parser.c
    lexer.c
    stmt.c
    semantics.c
    utils.c
)

add_executable(parser parse_file.c ${SOURCES} ${HEADERS})
add_executable(semChecker check_semantics.c ${SOURCES} ${HEADERS})

target_include_directories(parser PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(semChecker PRIVATE ${CMAKE_SOURCE_DIR}/include)
