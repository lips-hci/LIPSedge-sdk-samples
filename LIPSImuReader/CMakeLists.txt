### CMakeLists for LIPSImuReader ###

cmake_minimum_required (VERSION 3.1)

set(TARGET_SAMPLE_NAME LIPSImuReader)

project(${TARGET_SAMPLE_NAME})

## Setup Building Environment
if(NOT ENVIRONMENT_INITIALIZED)
    include(${PROJECT_SOURCE_DIR}/../CMake/SetupBuildEnvironment.cmake)
    set(ENVIRONMENT_INITIALIZED 1)
endif()

## Build Target Executable
add_executable(${TARGET_SAMPLE_NAME}
    Source/main.cpp
)

target_include_directories(${TARGET_SAMPLE_NAME} PRIVATE
    ${OPENNI2_INC}
    ${PROJECT_SOURCE_DIR}/../common
)

target_link_libraries(${TARGET_SAMPLE_NAME}
    ${OPENNI2_LIB}
)

set_property(TARGET ${TARGET_SAMPLE_NAME} PROPERTY CXX_STANDARD 11)

set_target_properties(${TARGET_SAMPLE_NAME} PROPERTIES FOLDER "LIPSedgeSDKSamples")

## Installation
install(TARGETS ${TARGET_SAMPLE_NAME} DESTINATION .)
