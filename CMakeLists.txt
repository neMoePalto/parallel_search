cmake_minimum_required(VERSION 3.5)

project(mtfind LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(sources
  src/async_task_runner.h
  src/command_line_args_handler.h
  src/async_task_runner.cpp
  src/command_line_args_handler.cpp
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
