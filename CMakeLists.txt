#This Cmake config file need the environment variable NACL_SDK_ROOT to be set on your "pepper_XX" directory

cmake_minimum_required(VERSION 2.8.4)
set(PNACL_PROGRAMS copy)
project(inputzone)

include( CMakeForceCompiler )


set( NACL                       ON )
set( PCH_DISABLE                ON )

set( PLATFORM_EMBEDDED          ON )
set( PLATFORM_NAME              "PNaCl" )
set( PLATFORM_TRIPLET           "pnacl" )
set( PLATFORM_PREFIX            "$ENV{NACL_SDK_ROOT}/toolchain/linux_pnacl" )
set( PLATFORM_PORTS_PREFIX      "${CMAKE_SOURCE_DIR}/ports/PNaCl" )
set( PLATFORM_EXE_SUFFIX        ".pexe" )

set( CMAKE_SYSTEM_NAME          "Linux" CACHE STRING "Target system." )
set( CMAKE_SYSTEM_PROCESSOR     "LLVM-IR" CACHE STRING "Target processor." )
set( CMAKE_FIND_ROOT_PATH       "${PLATFORM_PORTS_PREFIX};${PLATFORM_PREFIX}/usr" )
set( CMAKE_AR                   "${PLATFORM_PREFIX}/bin64/${PLATFORM_TRIPLET}-ar" CACHE STRING "" FORCE)
set( CMAKE_RANLIB               "${PLATFORM_PREFIX}/bin64/${PLATFORM_TRIPLET}-ranlib" CACHE STRING "" FORCE)
set( CMAKE_C_COMPILER           "${PLATFORM_PREFIX}/bin64/${PLATFORM_TRIPLET}-clang" )
set( CMAKE_CXX_COMPILER         "${PLATFORM_PREFIX}/bin64/${PLATFORM_TRIPLET}-clang++" )
set( CMAKE_C_FLAGS              "-U__STRICT_ANSI__" CACHE STRING "" )
set( CMAKE_CXX_FLAGS            "-U__STRICT_ANSI__" CACHE STRING "" )
set( CMAKE_C_FLAGS_RELEASE      "-O4 -ffast-math" CACHE STRING "" )
set( CMAKE_CXX_FLAGS_RELEASE    "-O4 -ffast-math" CACHE STRING "" )

cmake_force_c_compiler(         ${CMAKE_C_COMPILER} Clang )
cmake_force_cxx_compiler(       ${CMAKE_CXX_COMPILER} Clang )

set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )


macro( pnacl_finalise _target )
  add_custom_command( TARGET ${_target} POST_BUILD
    COMMENT "Finalising ${_target}"
    COMMAND "${PLATFORM_PREFIX}/bin64/${PLATFORM_TRIPLET}-finalize" "$<TARGET_FILE:${_target}>" --compress  )
endmacro()

macro( pnacl_nmf _target )
    add_custom_command( TARGET ${_target}
     POST_BUILD COMMAND "python" "$ENV{NACL_SDK_ROOT}/tools/create_nmf.py"
                        $<TARGET_FILE:${_target}> > $<TARGET_FILE:${_target}>.nmf VERBATIM)
endmacro()

include_directories( SYSTEM $ENV{NACL_SDK_ROOT}/include )
include_directories( SYSTEM $ENV{NACL_SDK_ROOT}/include/newlib )
include_directories( lib/IzLib/include )
link_directories( $ENV{NACL_SDK_ROOT}/lib/pnacl/Release )

set(
    CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_HOME_DIRECTORY}
    )

add_library(
    IzLib
    STATIC
    lib/IzLib/src/FileConverter.cc lib/IzLib/src/IzInstance.cc
    lib/IzLib/include/FileConverter.h lib/IzLib/include/IzInstance.h
)


foreach(PROGRAM ${PNACL_PROGRAMS})

message( "Building converter : ${SOURCE_FILES}" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web/format/${PROGRAM}/bin")
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/web/format/${PROGRAM}/*.cc)

add_executable(${PROGRAM} ${SOURCE_FILES})
set_target_properties(${PROGRAM} PROPERTIES OUTPUT_NAME ${PROGRAM}${PLATFORM_EXE_SUFFIX})

target_link_libraries(

        ${PROGRAM} IzLib

        ppapi_cpp ppapi_stub pthread nacl_io

)

pnacl_finalise(${PROGRAM})
pnacl_nmf(${PROGRAM})

endforeach()