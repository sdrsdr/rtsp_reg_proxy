cmake_minimum_required(VERSION 2.6)
project(rtsp_reg_proxy)

set(CMAKE_C_FLAGS_COMMON "-std=c99 -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_COMMON} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_COMMON} -O2")


add_executable(rtsp_reg_proxy rtsp_reg_proxy.c inettools.c urlparse.c strtoint.c buffer.c epollio.c)

#install(TARGETS rtsp_reg_proxy RUNTIME DESTINATION bin)
