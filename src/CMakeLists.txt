set(HEADERS
    ../include/exp.h
    ../include/interp.h
    ../include/ir.h
    ../include/parser.h
    ../include/reader.h
    ../include/semantics.h
    ../include/stmt.h
    ../include/structs.h
    ../include/utils.h
)

set(SOURCES
    exp.c   
    interp.c 
    parser.c
    reader.c
    stmt.c
    semantics.c
    utils.c
)

add_executable(parser parse_file.c ${SOURCES} ${HEADERS})
add_executable(semChecker check_semantics.c ${SOURCES} ${HEADERS})

target_include_directories(parser PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(semChecker PRIVATE ${CMAKE_SOURCE_DIR}/include)
