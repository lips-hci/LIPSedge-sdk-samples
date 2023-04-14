### Samples Building Environment Setup ###

## Initial Build Configuration Setup ##
if(NOT CONFIGURATIONS_INITIALIZED)
    set(CONFIGURATIONS_INITIALIZED 1)

    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build types")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

    ## Setup Build Configuration for Multiconfigs Generator
    if(CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available Solution Configurations" FORCE)
    else()
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
    endif()
endif()


## Check Build Type ##
if(NOT CMAKE_BUILD_TYPE)
    ## Set default build type to "Release" if it is not defined.
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build types" FORCE)
endif()
if(CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set(DEBUG 0)
elseif(CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(DEBUG 1)
else()
    message(FATAL_ERROR "Build mode not support! Only support RELEASE or DEBUG")
endif()


## Check Host Platform ##
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WINDOWS 1)
    if(MSVC)
        add_definitions(/DWINDOWS /D_WINDOWS /DPLAT_WINDOWS /DWIN32)
    endif()
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(HOST_ARCH x64)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(HOST_ARCH x86)
    endif ()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX 1)
    add_definitions(-DLINUX -DPLAT_LINUX -Dlinux)
    add_definitions(-DUNIX -DGLX_GLXEXT_LEGACY)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-no-pie")
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ".")
else()
    message(FATAL_ERROR "Host Platform NOT Support!")
endif()


## Set Output Folder ##
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Binary output directory path.")
if(WINDOWS)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/Bin CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    # Copy openni2 redist resources to Bin
    if(HOST_ARCH STREQUAL "x64")
        set(NI2_REDIST $ENV{OPENNI2_REDIST64})
    else()
        set(NI2_REDIST $ENV{OPENNI2_REDIST})
    endif()
    install(DIRECTORY ${NI2_REDIST}/ DESTINATION ${CMAKE_INSTALL_PREFIX})
elseif(LINUX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/Bin CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    # Copy openni2 redist resources to Bin
    set(NI2_REDIST $ENV{OPENNI2_REDIST})
    install(DIRECTORY ${NI2_REDIST}/ DESTINATION ${CMAKE_INSTALL_PREFIX})
else()
    message(FATAL_ERROR "Unknown Host Platform!")
endif()

## Locate OpenNI2 Header and Library Path ##
if(WINDOWS)
    if(HOST_ARCH STREQUAL "x64")
        find_path( OPENNI2_INC NAMES OpenNI.h HINTS
            "$ENV{OPENNI2_INCLUDE64}"
            DOC "OpenNI2 Include Header Directory"
            NO_DEFAULT_PATH
        )
        find_library( OPENNI2_LIB NAMES OpenNI2 HINTS
            "$ENV{OPENNI2_LIB64}"
            DOC "OpenNI2 Linking Library Directory"
            NO_DEFAULT_PATH
        )
    else()
        find_path( OPENNI2_INC NAMES OpenNI.h HINTS
            "$ENV{OPENNI2_INCLUDE}"
            DOC "OpenNI2 Include Header Directory"
            NO_DEFAULT_PATH
        )
        find_library( OPENNI2_LIB NAMES OpenNI2 HINTS
            "$ENV{OPENNI2_LIB}"
            DOC "OpenNI2 Linking Library Directory"
            NO_DEFAULT_PATH
        )
    endif()
elseif(LINUX)
    find_path( OPENNI2_INC NAMES OpenNI.h HINTS
        "$ENV{OPENNI2_INCLUDE}"
        DOC "OpenNI2 Include Header Directory"
        NO_DEFAULT_PATH
    )
    find_library( OPENNI2_LIB NAMES OpenNI2 HINTS
        "$ENV{OPENNI2_REDIST}"
        DOC "OpenNI2 Linking Library Directory"
        NO_DEFAULT_PATH
    )
else()
    message(FATAL_ERROR "Unknown Host Platform!")
endif()

if(NOT OPENNI2_INC)
message(STATUS "====================================================================================")
message(STATUS "Cannot determine path for OPENNI2_INC because OPENNI2_INCLUDE is not defined.")
message(STATUS "Please define it or 'source' the OpenNIDevEnvironment file from the SDK directory.")
message(STATUS "")
message(STATUS "$ cd OpenNI2")
message(STATUS "$ source OpenNIDevEnvironment")
message(STATUS "====================================================================================")
message(FATAL_ERROR "OPENNI2_INC is NOT FOUND!")
endif()

if(NOT OPENNI2_LIB)
message(STATUS "====================================================================================")
message(STATUS "Cannot determine path for OPENNI2_LIB because OPENNI2_REDIST is not defined.")
message(STATUS "Please define it or 'source' the OpenNIDevEnvironment file from the SDK directory.")
message(STATUS "")
message(STATUS "$ cd OpenNI2")
message(STATUS "$ source OpenNIDevEnvironment")
message(STATUS "====================================================================================")
message(FATAL_ERROR "OPENNI2_LIB is NOT FOUND!")
endif()

message(STATUS "\n==================================================================================")
message(STATUS "CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
message(STATUS "CMAKE_SOURCE_DIR = " ${CMAKE_SOURCE_DIR})
message(STATUS "CMAKE_BINARY_DIR = " ${CMAKE_BINARY_DIR})
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY = " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(STATUS "CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX})
message(STATUS "OPENNI2_INC = " ${OPENNI2_INC})
message(STATUS "OPENNI2_LIB = " ${OPENNI2_LIB})
message(STATUS "====================================================================================\n")
