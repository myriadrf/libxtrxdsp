cmake_minimum_required(VERSION 2.8)
project(libxtrxdsp C)

# Set the version information here
set(MAJOR_VERSION 0)
set(API_COMPAT    0)
set(MINOR_VERSION 1)
set(MAINT_VERSION git)

set(LIBVER  "${MAJOR_VERSION}.${API_COMPAT}.${MINOR_VERSION}")

# determine target architecture
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE CC_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
if(CC_ARCH MATCHES "i386")
    message(STATUS "Building for i386")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
    set(ARCH "x86")
elseif(CC_ARCH MATCHES "x86_64")
    message(STATUS "Building for x86_64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(ARCH "x86")
elseif(CC_ARCH MATCHES "aarch64")
    message(STATUS "Building for aarch64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    set(ARCH "arm")
elseif(CC_ARCH MATCHES "arm.*eabihf")
    message(STATUS "Building for armhf")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
    set(ARCH "arm")
else()
    message(WARNING "Unknown target architecture ${CC_ARCH}, forcing to FORCE_ARCH=${FORCE_ARCH} variable if it was provided")
    set(ARCH ${FORCE_ARCH})
endif()

set(CROSS_COMPILE_INCLUDE_PATH "/usr/${CC_ARCH}/include")
set(CROSS_COMPILE_LIB_PATH     "/usr/${CC_ARCH}/lib")


if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SYSTEM_LIBS rt dl pthread)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(SYSTEM_LIBS "")
    if(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_SHARED_LINKER_FLAGS " -Wl,--kill-at -static-libgcc ")
    endif(CMAKE_COMPILER_IS_GNUCC)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(SYSTEM_LIBS rt dl pthread)
else()
    message(WARNING "Unknown system type is ${CMAKE_SYSTEM_NAME}")
endif()

add_definitions(-Wall -g)

set(XTRXDSP_LIBRARY_DIR      lib${LIB_SUFFIX})
set(XTRXDSP_INCLUDE_DIR      include)
set(XTRXDSP_UTILS_DIR        ${XTRXDSP_LIBRARY_DIR}/xtrxdsp)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/libxtrxdsp.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libxtrxdsp.pc
@ONLY)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libxtrxdsp.pc
    DESTINATION ${XTRXDSP_LIBRARY_DIR}/pkgconfig
)

# Report all undfined symbols in shared libraries
if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions("-Wl,-z,defs")
endif(CMAKE_COMPILER_IS_GNUCC)

########################################################################
# Build DSP library
########################################################################
set(GENERIC_TUNE "")

include_directories(".")

set(XTRX_DSP_FILES xtrxdsp.c xtrxdsp_fft.c xtrxdsp_filters.c xtrxdsp_filters_data.c xtrxdsp_no.c)
if(ARCH MATCHES "^x86.*")
    set(XTRX_DSP_FILES ${XTRX_DSP_FILES}
                       xtrxdsp_x86_sse2.c
                       xtrxdsp_x86_avx.c
                       xtrxdsp_x86_avx_fma.c)

    set_source_files_properties(xtrxdsp_x86_sse2.c     PROPERTIES COMPILE_FLAGS "-O3 -msse2")
    set_source_files_properties(xtrxdsp_x86_avx.c      PROPERTIES COMPILE_FLAGS "-O3 -mavx")
    set_source_files_properties(xtrxdsp_x86_avx_fma.c  PROPERTIES COMPILE_FLAGS "-O3 -mavx -mfma")
endif()

add_library(xtrxdsp SHARED ${XTRX_DSP_FILES})

set_source_files_properties(xtrxdsp.c              PROPERTIES COMPILE_FLAGS "-O3 ${GENERIC_TUNE}")
set_source_files_properties(xtrxdsp_x86_no.c       PROPERTIES COMPILE_FLAGS "-O3 ${GENERIC_TUNE}")
set_target_properties(xtrxdsp PROPERTIES VERSION ${LIBVER} SOVERSION ${MAJOR_VERSION})


add_subdirectory(tests)

########################################################################
# install headers & targets
########################################################################
install(TARGETS xtrxdsp DESTINATION ${XTRXDSP_LIBRARY_DIR})

install(FILES
    xtrxdsp.h xtrxdsp_config.h xtrxdsp_filters.h xtrxdsp_fft.h
    DESTINATION ${XTRXDSP_INCLUDE_DIR}
)

########################################################################
# package generator
########################################################################
if(NOT CPACK_GENERATOR)
    set(CPACK_GENERATOR DEB)
endif()
set(CPACK_PACKAGE_NAME                "libxtrxdsp")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "XTRXDSP library")
set(CPACK_PACKAGE_VENDOR              "Fairwaves, Inc.")
set(CPACK_PACKAGE_CONTACT             "http://fairwaves.co/wp/contact-us/")
set(CPACK_PACKAGE_VERSION             ${LIBVER}-1)
set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libc6 (>= 2.17)")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
set(CPACK_SET_DESTDIR               "")
set(CPACK_PACKAGING_INSTALL_PREFIX  "/usr")
set(CPACK_PACKAGE_CONTACT           "Sergey Kostanbaev <sergey.kostanbaev@fairwaves.co>")

include(CPack) #include last
