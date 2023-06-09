# mkdir build
# cd build
# cmake ../ -DBISON_DEBUG:STRING="--debug" # For debugging Bison
# cmake ../ -DBISON_DEBUG:STRING="" # To disable Bison debugging
# make -j$(nproc) SHEBLANG_compiler # Or SHEBLANG_interpreter

cmake_minimum_required(VERSION 3.0.0)
project(SHEBLANG VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS TRUE)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -flto")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS "-stdlib=libstdc++ -Wno-write-strings -Wno-format-security -ferror-limit=5")
set(CMAKE_CXX_COMPILER "clang++-16")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY          ${PROJECT_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(SRC_DIR "./")
set(PARSER_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LEXER_OUT "${PARSER_DIR}/lexer.cpp")
set(PARSER_OUT "${PARSER_DIR}/parser.cpp")

flex_target(LEXER "${SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/lexer.h")
# Add ` COMPILE_FLAGS "--debug"` if needed.
bison_target(PARSER "${SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${PARSER_DIR}/parser.h" COMPILE_FLAGS "-Wcounterexamples")

add_flex_bison_dependency(LEXER PARSER)

# Source files shared between the compiler and interpreter
set(SHARED_SOURCES
  cls.cpp
  node.cpp
  nodes.cpp
  node_constructors.cpp
  semantic_utils.cpp
  setup_scopes.cpp
  value_operators.cpp
  const_expr_checker.cpp
  ${LEXER_OUT}
  ${PARSER_OUT}
)

set(COMPILER_SOURCES
  clc.cpp
  ${SHARED_SOURCES}
)

set(INTERPRETER_SOURCES
  cli.cpp
  ${SHARED_SOURCES}
)

include_directories(
  ./
)

add_executable(${PROJECT_NAME}_compiler ${COMPILER_SOURCES})
add_executable(${PROJECT_NAME}_interpreter ${INTERPRETER_SOURCES})
