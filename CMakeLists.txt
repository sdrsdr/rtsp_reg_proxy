cmake_minimum_required(VERSION 2.6)
project(rtsp_reg_proxy)

add_executable(rtsp_reg_proxy main.cpp)

install(TARGETS rtsp_reg_proxy RUNTIME DESTINATION bin)
