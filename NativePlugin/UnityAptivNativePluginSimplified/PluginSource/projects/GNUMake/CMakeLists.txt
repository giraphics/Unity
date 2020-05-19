cmake_minimum_required(VERSION 3.5)

set(ProjectName RenderingPlugin)
project(${ProjectName})

############################################################
# Create a library
############################################################

#Generate the shared library from the library sources
add_library(${ProjectName} SHARED 
    ../../source/Unity/IUnityGraphics.h
    ../../source/Unity/IUnityGraphicsVulkan.h
    ../../source/Unity/IUnityInterface.h
    ../../source/RenderAPI.h
    ../../source/RenderAPI.cpp
    ../../source/RenderAPI_Vulkan.cpp
    ../../source/RenderingPlugin.cpp
)
add_library(hello::library ALIAS ${ProjectName})

target_include_directories(${ProjectName}
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/include
)
