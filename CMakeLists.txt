cmake_minimum_required(VERSION 3.21)
project(RAT_Replay_Parser)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(~/.cache/node-gyp/14.15.4/include)

add_executable(RAT_Replay_Parser src/replay.cc)