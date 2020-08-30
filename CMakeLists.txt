cmake_minimum_required(VERSION 3.5)
project(soku-events)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add extra libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty)

# Force WebSocket++ to use 100% C++11 mode, so that it doesn't try to look for Boost
add_definitions(-D_WEBSOCKETPP_CPP11_STRICT_)
add_definitions(-DASIO_STANDALONE)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty/websocketpp)
include_directories(${ASIO_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(soku-events MODULE src/main.cpp)
target_link_libraries(soku-events shlwapi -static)
#set_target_properties(soku-events PROPERTIES PREFIX "")

add_custom_command(TARGET soku-events POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:soku-events> ${PROJECT_SOURCE_DIR}/dist/${targetfile}
)
