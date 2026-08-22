cmake_minimum_required(VERSION 3.14)
project(cppx86 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings and sanitizers
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
# Enable sanitizers for debug builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(include)

# For now, collect sources
set(COMMON_SOURCES
    # src/diagnostics/DiagnosticEngine.cpp
)

set(LEXER_SOURCES
    src/lexer/Lexer.cpp
)

set(PARSER_SOURCES
    src/parser/Parser.cpp
)

set(SEMA_SOURCES
    src/sema/SemanticAnalyzer.cpp
)

add_library(cppx86_lib STATIC
    ${COMMON_SOURCES}
    ${LEXER_SOURCES}
    ${PARSER_SOURCES}
    ${SEMA_SOURCES}
)

# Compiler driver
add_executable(cppx86
    src/driver/main.cpp
)
target_link_libraries(cppx86 cppx86_lib)

# Testing using GoogleTest
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(cppx86_tests
    tests/lexer/LexerTest.cpp
)
target_link_libraries(cppx86_tests GTest::gtest_main cppx86_lib)

include(GoogleTest)
gtest_discover_tests(cppx86_tests)
