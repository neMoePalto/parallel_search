cmake_minimum_required(VERSION 3.5)

project(mtfind LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(sources
  src/functions.h
  src/functions.cpp
  src/main.cpp
)

find_package(Boost COMPONENTS regex REQUIRED)

add_executable(mtfind
  ${sources}
)

target_link_libraries(mtfind
  Boost::regex
  pthread
)
