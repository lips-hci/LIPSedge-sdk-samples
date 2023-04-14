### Setup Linking Path to OpenCV ###

## Set OpenCV Version ##
if(WINDOWS AND MSVC AND MSVC_VERSION LESS 1900)
    ## In Windows, OpenCV 3.4 not support Visual Studio version older than 2015
    set(USE_CV3 OFF CACHE BOOL "Build with OpenCV3.")
else()
    set(USE_CV3 ON CACHE BOOL "Build with OpenCV3.")
endif()

if(USE_CV3)
    set(OPENCV_VER_REQ 3.4)
else()
    set(OPENCV_VER_REQ 2.4)
endif()


## Locate OpenCV Header and Library Path ##
if(WINDOWS)
    set(OpenCV_DIR "C:/opencv/build" CACHE PATH "Directory path to search for OpenCVConfig.cmake")

    if(NOT USE_CV3)
        ## Use static library in Winodws for OpenCV2
        ## Since OpenCV3 doesn't provide static library in officially prebuilt.
        set(OpenCV_STATIC ON)
        if(MSVC)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
        endif()
    else()
        if(MSVC)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
        endif()
    endif()
elseif(LINUX)
    set(OpenCV_DIR "/usr/local/share/OpenCV" CACHE PATH "Directory path to search for OpenCVConfig.cmake")
else()
    message(FATAL_ERROR "Unknown Host Platform!")
endif()

message(STATUS "\nSearch OpenCVConfig.cmake in " ${OpenCV_DIR})
message(STATUS "If you would like to change the searching folder, please modify the value of OpenCV_DIR.\n")
find_package(OpenCV ${OPENCV_VER_REQ} REQUIRED)

if(OpenCV_FOUND)
    set(OPENCV_VER_FOUND ${OpenCV_VERSION})
    set(OPENCV_INC ${OpenCV_INCLUDE_DIRS})
    set(OPENCV_LIB ${OpenCV_LIBS})
endif()


message(STATUS "\n====================================================================================================")
message(STATUS "OpenCV_DIR = " ${OpenCV_DIR})

if(OpenCV_FOUND)
    message(STATUS "OPENCV_VER = " ${OPENCV_VER_FOUND})
    message(STATUS "OPENCV_INC = " ${OPENCV_INC})
    message(STATUS "OPENCV_LIB = " ${OPENCV_LIB})
else()
    message(STATUS "OpenCV Version Required: " ${OPENCV_VER_REQ})
    message(SEND_ERROR "OpenCV NOT Found!")
endif()
message(STATUS "====================================================================================================\n")
