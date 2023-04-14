### Setup Linking Path to glfw ###
macro(build_glfw glfw_src_path)
    include(ExternalProject)
    ExternalProject_Add(
        glfw
        SOURCE_DIR ${glfw_src_path}
        BINARY_DIR ${CMAKE_BINARY_DIR}/third-party/glfw
        PREFIX ${CMAKE_BINARY_DIR}/third-party/glfw
        CMAKE_ARGS
        -DCMAKE_CXX_STANDARD_LIBRARIES=${CMAKE_CXX_STANDARD_LIBRARIES}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third-party/glfw
        -DGLFW_BUILD_EXAMPLES=FALSE
        -DGLFW_BUILD_TESTS=FALSE
        -DGLFW_VULKAN_STATIC=FALSE
        -DBUILD_SHARED_LIBS=FALSE
    )

    set(GLFW_INC ${CMAKE_BINARY_DIR}/third-party/glfw/include)
    set(GLFW_DEPS_INC ${glfw_src_path}/deps)

    if(WINDOWS)
        set(GLFW_LIB ${CMAKE_BINARY_DIR}/third-party/glfw/lib/glfw3.lib glu32.lib opengl32.lib)
    elseif(LINUX)
        set(GLFW_LIB ${CMAKE_BINARY_DIR}/third-party/glfw/lib/libglfw3.a -lpthread -lm -lz -lGL -lGLU -lX11 -lXext -lXfixes -ldl)
    else()
        message(FATAL_ERROR "Unknown Host Platform!")
    endif()
endmacro(build_glfw)
