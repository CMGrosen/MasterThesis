cmake_minimum_required(VERSION 2.8)

project("Antlr-cpp-tutorial")

set(CMAKE_EXE_LINKER_FLAGS "-isystem include")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CIMG_CFLAGS}")

# CMake commands for this specific project start here

# to disable some warnings for CImg
set(CMAKE_CXX_FLAGS  "-std=c++17")
if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "-Wdeprecated -Wno-attributes -Wall -Wextra -std=c++17" )
endif()

if (APPLE)
    set(CMAKE_CXX_FLAGS "-Wdeprecated -Wall -Wextra -std=c++17" )
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc /wd4127 /wd4311 /wd4312 /wd4512 /wd4571 /wd4640 /wd4706 /wd4710 /wd4800 /wd4804 /wd4820 /wd4996")
endif()


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
        ${PROJECT_SOURCE_DIR}/z3-4.8.7/bin/
        ${PROJECT_SOURCE_DIR}/z3-4.8.7/include/
)

set(visitor_SRC
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${PROJECT_SOURCE_DIR}/src/transformers/DST.cpp
        ${visitor-GENERATED_SRC}
        src/nodes/nodes.hpp src/nodes/edge.cpp src/nodes/edge.hpp src/nodes/basicblock.cpp src/nodes/basicblock.hpp src/symengine/symEngine.cpp src/symengine/symEngine.hpp src/symengine/trace.cpp src/symengine/trace.hpp src/nodes/expressions/readNode.cpp src/symengine/interpreter.cpp src/symengine/interpreter.hpp src/symengine/VariableValue.hpp src/symengine/Difference.hpp src/symengine/state.cpp src/symengine/state.hpp src/symengine/statesHandler.cpp src/symengine/statesHandler.hpp src/symengine/intepreterData.hpp src/symengine/value.hpp src/nodes/statements/option.hpp src/CFGs/SSA_CCFG.hpp src/CFGs/CSSA_CCFG.hpp src/CFGs/CCFG.hpp)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set (flags_1 "-Wno-overloaded-virtual")
else()
    set (flags_1 "-MP /wd4251")
endif()

foreach( src_file ${visitor_SRC} )
    set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "${COMPILE_FLAGS} ${flags_1}"
    )
endforeach( src_file ${visitor_SRC} )

add_executable(visitor ${visitor_SRC})

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # see https://github.com/antlr/antlr4/issues/1874
    target_compile_options(visitor PUBLIC "$<$<CONFIG:DEBUG>:/MT>")
endif()

if(WIN32)
    target_link_libraries(visitor "${PROJECT_SOURCE_DIR}/libs/antlr4-runtime.lib")
endif()

if(APPLE)
    target_link_libraries(visitor "${PROJECT_SOURCE_DIR}/libs/libantlr4-runtime.dylib")
    target_link_libraries(visitor "${PROJECT_SOURCE_DIR}/z3-4.8.7/bin/osx/libz3.dylib")
elseif(UNIX)
    target_link_libraries(visitor "${PROJECT_SOURCE_DIR}/libs/libantlr4-runtime.so")
    target_link_libraries(visitor "${PROJECT_SOURCE_DIR}/z3-4.8.7/bin/linux/libz3.so")
endif()

target_link_libraries(visitor ${SYSTEM_LIBS})

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_custom_command(TARGET visitor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/libs/libantlr4-runtime.so"
            $<TARGET_FILE_DIR:visitor>)
endif()

if(WIN32)
    add_custom_command(TARGET visitor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/libs/antlr4-runtime.dll"
            $<TARGET_FILE_DIR:visitor>)
endif()

if(APPLE)
    add_custom_command(TARGET visitor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/libs/libantlr4-runtime.dylib"
            $<TARGET_FILE_DIR:visitor>)
endif()

# copy the input file next to the generated executable
#[[add_custom_command(TARGET visitor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/code.small"
        $<TARGET_FILE_DIR:visitor>)
]]
# necessary because the default working directory of Visual Studio
# is not the same as the one in which the binary is created
# so we copy the input file twice:
# one for when you launch the executale in Visual Studio (CTRL+F5)
# one for when you launch it normally
if (WIN32)
    add_custom_command(TARGET visitor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/code.small"
            $<TARGET_FILE_DIR:visitor>/../)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
