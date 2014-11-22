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
set( CMAKE_CXX_FLAGS            "-Wc++11-extensions" CACHE STRING "" )
set( CMAKE_C_FLAGS_RELEASE      "-O4 -ffast-math" CACHE STRING "" )
set( CMAKE_CXX_FLAGS_RELEASE    "-O4 -ffast-math" CACHE STRING "" )
set( CMAKE_C_FLAGS_DEBUG     "-g  " CACHE STRING "" )
set( CMAKE_CXX_FLAGS_DEBUG    "-g " CACHE STRING "" )

cmake_force_c_compiler(         ${CMAKE_C_COMPILER} Clang )
cmake_force_cxx_compiler(       ${CMAKE_CXX_COMPILER} Clang )

set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

#Used for debugging
macro( pnacl_copy_llvm_bitcode _target )
add_custom_command(TARGET ${_target} POST_BUILD
 COMMENT "Copying ${_target}.bc to bin for debug"
 COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}
 COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${_target}> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROGRAM}.bc)
endmacro()

macro( pnacl_finalise _target )
  add_custom_command( TARGET ${_target} POST_BUILD
    COMMENT "Finalising ${_target}"
    COMMAND "${PLATFORM_PREFIX}/bin64/${PLATFORM_TRIPLET}-finalize" "$<TARGET_FILE:${_target}>" --compress  )
endmacro()

macro( pnacl_nmf _target )

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_custom_command( TARGET ${_target}
         POST_BUILD COMMAND "python" "$ENV{NACL_SDK_ROOT}/tools/create_nmf.py" $<TARGET_FILE:${_target}> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROGRAM}.bc > $<TARGET_FILE:${_target}>.nmf --pnacl-debug-optlevel=0 VERBATIM)
    else()
        add_custom_command( TARGET ${_target}
        POST_BUILD COMMAND "python" "$ENV{NACL_SDK_ROOT}/tools/create_nmf.py" $<TARGET_FILE:${_target}> > $<TARGET_FILE:${_target}>.nmf VERBATIM)
    endif(CMAKE_BUILD_TYPE MATCHES "Debug")
endmacro()

include_directories( SYSTEM $ENV{NACL_SDK_ROOT}/include SYSTEM $ENV{NACL_SDK_ROOT}/include/newlib lib/pnacl/IzLib/include lib/pnacl/thirdparty/include )
link_directories( $ENV{NACL_SDK_ROOT}/lib/pnacl/Release ${CMAKE_SOURCE_DIR}/lib/pnacl/thirdparty/lib)

set(
    CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_HOME_DIRECTORY}
    )

add_library(
    IzLib
    STATIC
    lib/pnacl/IzLib/src/FileConverter.cc lib/pnacl/IzLib/src/IzInstance.cc lib/pnacl/IzLib/src/InputDownloader.cc
    lib/pnacl/IzLib/include/FileConverter.h lib/pnacl/IzLib/include/IzInstance.h lib/pnacl/IzLib/include/InputDownloader.h
)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web/converters/webp/bin")
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/web/converters/webp/pnacl/shared/*)



#MESSAGE(POST_BUILD COMMAND "python" "$ENV{NACL_SDK_ROOT}/tools/create_nmf.py" $<TARGET_FILE:${_target}> $<$<CONFIG:Debug>:"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cwebp.bc"> > $<TARGET_FILE:${_target}>.nmf
#                                $<$<CONFIG:Debug>:--pnacl-debug-optlevel=0> VERBATIM)
set(PROGRAM cwebp)
add_executable(${PROGRAM} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/web/converters/webp/pnacl/${PROGRAM}Module.cc)
set_target_properties(${PROGRAM} PROPERTIES OUTPUT_NAME ${PROGRAM}${PLATFORM_EXE_SUFFIX})
set_target_properties(${PROGRAM} PROPERTIES COMPILE_DEFINITIONS "WEBP_HAVE_JPEG=1;WEBP_HAVE_PNG=1;WEBP_HAVE_TIFF=1")
target_link_libraries(

        ${PROGRAM} IzLib

        ppapi_cpp ppapi_stub pthread nacl_io jpeg png z tiff webp

)

pnacl_finalise(${PROGRAM})
pnacl_nmf(${PROGRAM})

set(PROGRAM dwebp)
add_executable(${PROGRAM} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/web/converters/webp/pnacl/${PROGRAM}Module.cc)
set_target_properties(${PROGRAM} PROPERTIES OUTPUT_NAME ${PROGRAM}${PLATFORM_EXE_SUFFIX})
set_target_properties(${PROGRAM} PROPERTIES COMPILE_DEFINITIONS "WEBP_HAVE_PNG=1;WEBP_HAVE_TIFF=1")
target_link_libraries(

        ${PROGRAM} IzLib

        ppapi_cpp ppapi_stub pthread nacl_io png z tiff webp

)

pnacl_finalise(${PROGRAM})
pnacl_nmf(${PROGRAM})
