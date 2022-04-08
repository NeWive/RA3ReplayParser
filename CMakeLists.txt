cmake_minimum_required(VERSION 3.21)
project(RAT_Replay_Parser)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}\\include)

add_executable(RAT_Replay_Parser
        main.cpp RA3Replay.cpp RA3Replay.h)