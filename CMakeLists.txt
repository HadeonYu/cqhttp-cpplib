cmake_minimum_required(VERSION 3.5)
project(cqhttp-cpplib VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(SOURCES src/Bot.cpp src/Msg.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

add_subdirectory(include/json)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC SPDLOG_FMT_EXTERNAL
  PUBLIC FMT_HEADER_ONLY)
