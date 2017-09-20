cmake_minimum_required (VERSION 3.2)

project(stateful_pointer CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.61 REQUIRED)
find_path(BENCHMARK_INCLUDE_DIRS benchmark/Benchmark.h)
find_library(BENCHMARK_LIBRARY benchmark)

include_directories(../include ${Boost_INCLUDE_DIRS})

# tests
enable_testing()

file(GLOB TEST_SOURCES ../test/test_*.cpp ../examples/*.cpp)
foreach(SRC IN ITEMS ${TEST_SOURCES})
  if(SRC MATCHES "/([_a-zA-Z0-9]+)\\.cpp")
    add_executable(${CMAKE_MATCH_1} ${SRC})
    target_compile_options(${CMAKE_MATCH_1} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-O0 -g>)
    add_test(${CMAKE_MATCH_1} ${CMAKE_MATCH_1})
  endif()
endforeach()

# benchmarks
if (${BENCHMARK_INCLUDE_DIRS} AND ${BENCHMARK_LIBRARY})
  include_directories(${BENCHMARK_INCLUDE_DIRS})
  file(GLOB TEST_SOURCES ../test/bm_*.cpp)
  foreach(SRC IN ITEMS ${TEST_SOURCES})
    if(SRC MATCHES "/([_a-zA-Z0-9]+)\\.cpp")
      foreach(L 0 3)
        add_executable(${CMAKE_MATCH_1}_o${L} ${SRC})
        target_compile_options(${CMAKE_MATCH_1}_o${L} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-O${L}>)
        target_link_libraries(${CMAKE_MATCH_1}_o${L} ${BENCHMARK_LIBRARY} -lpthread)
      endforeach()
    endif()
  endforeach()
else()
  message(STATUS "Google Benchmark not found, benchmarks are not build")
endif()

# for clang format and clang tidy
file(GLOB_RECURSE
     ALL_SOURCE_FILES
     LIST_DIRECTORIES false
     ../test/*_test.cpp ../include/*.hpp
     )

add_custom_target(clf
  COMMAND clang-format
  -i
  ${ALL_SOURCE_FILES}
  )

get_property(INCLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
set(TIDY_INCLUDE)
foreach(x ${INCLUDE_DIRS})
  LIST(APPEND TIDY_INCLUDE -I${x})
endforeach()
add_custom_target(
  clang-tidy
  COMMAND clang-tidy
  ${ALL_SOURCE_FILES}
  -checks=*,-*alpha*
  --
  -std=c++11
  ${TIDY_INCLUDE}
  )
