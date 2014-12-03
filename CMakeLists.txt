cmake_minimum_required(VERSION 2.8)


project(inputzone)

#include(cmake/emscripten.cmake)
#include(cmake/pnacl.cmake)

#include(cmake/pnacl_webp.cmake)
include(cmake/pnacl_ffmpeg.cmake)

#include(lib/pnacl/thirdparty/libvpx.cmake)
#include(lib/pnacl/thirdparty/libwebp.cmake)
include(lib/pnacl/thirdparty/ffmpeglib.cmake)