#[[ cmake_minimum_required(VERSION 3.12)
project(visitor)

set(CMAKE_CXX_STANDARD 14)

set(visitor-GENERATED_SRC
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/SmallLexer.cpp
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/SmallParser.cpp
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/SmallBaseVisitor.cpp
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/SmallVisitor.cpp
        )

foreach( src_file ${visitor-GENERATED_SRC} )
    set_source_files_properties(
            ${src_file}
            PROPERTIES
            GENERATED TRUE
    )
endforeach( src_file ${visitor-GENERATED_SRC} )

include_directories(
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/misc
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/atn
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/dfa
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/tree
        ${PROJECT_SOURCE_DIR}/antlr4-runtime/support
        ${PROJECT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/libs/
)


add_executable(visitor main.cpp)
]]

# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

LIST( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

# compiler must be 11 or 14
SET (CMAKE_CXX_STANDARD 14)

# set variable pointing to the antlr tool that supports C++
set(ANTLR4CPP_JAR_LOCATION ${PROJECT_SOURCE_DIR}/thirdparty/antlr/antlr-4.7.2-complete.jar)
# add external build for antlrcpp
include( ExternalAntlr4Cpp )
# add antrl4cpp artifacts to project environment
include_directories( ${ANTLR4CPP_INCLUDE_DIRS} )
link_directories( ${ANTLR4CPP_LIBS} )
message(STATUS "Found antlr4cpp libs: ${ANTLR4CPP_LIBS} and includes: ${ANTLR4CPP_INCLUDE_DIRS} ")

# Call macro to add lexer and grammar to your build dependencies.
antlr4cpp_process_grammar(visitor antlrcpptest
        ${CMAKE_CURRENT_SOURCE_DIR}/Small.g4
        ${CMAKE_CURRENT_SOURCE_DIR}/tokens.g4)
# include generated files in project environment
include_directories(${antlr4cpp_include_dirs_antlrcpptest})

# add generated grammar to demo binary target
add_executable(visitor main.cpp ${antlr4cpp_src_files_antlrcpptest})
add_dependencies(visitor antlr4cpp antlr4cpp_generation_antlrcpptest)
target_link_libraries(visitor antlr4-runtime)
