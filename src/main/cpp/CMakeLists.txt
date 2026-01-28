cmake_minimum_required(VERSION 3.18.1)

project("mobileglues-renderer")

# Create the library
add_library(
        mobileglues-renderer
        SHARED
        renderer/Renderer.cpp
        renderer/StateManager.cpp
        renderer/Batcher.cpp
        renderer/ShaderCache.cpp
        renderer/JNI_Interface.cpp
)

# Search for NDK libraries
find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)

# Link libraries
target_link_libraries(
        mobileglues-renderer
        ${log-lib}
        ${android-lib}
        ${gles3-lib}
)

# Include directories
target_include_directories(mobileglues-renderer PRIVATE renderer)
