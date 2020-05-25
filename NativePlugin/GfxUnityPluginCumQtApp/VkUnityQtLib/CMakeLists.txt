cmake_minimum_required(VERSION 3.7.1)

set(Project_Name "GfxUnityQtLib")
set(Unity_Plugin_Lib "AptiviewUnityPlugin")
set(Qt_Binary "QtBinaryApp")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # 
# Include Qt basic functions
include(QtCommon)
# Set PROJECT_VERSION_PATCH and PROJECT_VERSION_TWEAK to 0 if not present, needed by add_project_meta
fix_project_version()

# Specify a suitable project name
project(${Project_Name}  VERSION 1.0.0)
add_project_meta(META_FILES_TO_INCLUDE)

#set(CMAKE_PREFIX_PATH "/home/parminder/.conan/data/Qt/5.14.0/parm2/qt5140/package/5da5c25536bf05f471c51e8e2b3ed02894d94b2e/lib/cmake")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_PREFIX_PATH "/Users/parminder/Dev/Software/QtBaseSrc/qtbase/lib/cmake")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_PREFIX_PATH "/home/parminder/.conan/data/Qt/5.14.0/parm2/qt5140/package/5da5c25536bf05f471c51e8e2b3ed02894d94b2e/lib/cmake")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
ELSE()
ENDIF()

find_package(Qt5Widgets REQUIRED)

OPTION(USE_D2D_WSI "Build the project using Direct to Display swapchain" OFF)
OPTION(USE_WAYLAND_WSI "Build the project using Wayland swapchain" OFF)

set(RESOURCE_INSTALL_DIR "" CACHE PATH "Path to install resources to (leave empty for running uninstalled)")

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

IF(WIN32)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSE(WIN32)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	find_package(Threads REQUIRED)
IF(USE_D2D_WSI)
	MESSAGE("Using direct to display extension...")
	add_definitions(-D_DIRECT2DISPLAY)
ELSEIF(USE_WAYLAND_WSI)
	find_program(PKG_CONFIG pkg-config)
	if (NOT PKG_CONFIG)
		message(FATAL_ERROR "pkg-config binary not found")
	endif ()
	find_package(Wayland REQUIRED)
	if (NOT WAYLAND_FOUND)
		message(FATAL_ERROR "Wayland development package not found")
	endif ()
	pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
	if (NOT WAYLAND_PROTOCOLS_FOUND)
		message(FATAL_ERROR "Wayland protocols package not found")
	endif ()
	find_program(WAYLAND_SCANNER wayland-scanner)
	if (NOT WAYLAND_SCANNER)
		message(FATAL_ERROR "wayland-scanner binary not found")
	endif ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
	include_directories(${WAYLAND_INCLUDE_DIR})
	execute_process(COMMAND ${PKG_CONFIG} --variable=pkgdatadir wayland-protocols OUTPUT_VARIABLE protocol_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
			COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
	include_directories(${CMAKE_BINARY_DIR})
ELSE(USE_D2D_WSI)
	find_package(XCB REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
ENDIF(USE_D2D_WSI)
	# Todo : android?
ENDIF(WIN32)

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)
# GLM SETUP - Mathematic libraries for 3D transformation
set(GLMINCLUDES "${GLM_DIR}")
get_filename_component(GLMINC_PREFIX "${GLMINCLUDES}" ABSOLUTE)
if(NOT EXISTS ${GLMINC_PREFIX})
    message(FATAL_ERROR "Necessary glm headers do not exist: " ${GLMINC_PREFIX})
endif()
include_directories( ${GLMINC_PREFIX} )

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Clang specific stuff
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB_RECURSE COMMON_BASE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/base/*.*)

#file(GLOB_RECURSE UNITY_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/NativeUntyTriangleCode/PluginSource/source/*.*)
#file(GLOB_RECURSE QT_UNITY_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/examples/NativeUntyTriangleCode/QtUnityTriangleImpl/*.*)
#add_library(${Unity_Plugin_Lib} SHARED ${COMMON_BASE_FILES} ${RESOURCE_FILES}  ${UNITY_SOURCE_FILES} ${QT_UNITY_IMPL})

file(GLOB_RECURSE UNITY_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/NativeUntyDynamicuniformbuffer/PluginSource/source/*.*)
file(GLOB_RECURSE QT_UNITY_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/examples/NativeUntyDynamicuniformbuffer/QtUnityDynamicUniformBufferImpl/*.*)
add_library(${Unity_Plugin_Lib} SHARED ${COMMON_BASE_FILES} ${RESOURCE_FILES}  ${UNITY_SOURCE_FILES} ${QT_UNITY_IMPL})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base)

# Link the debug and release libraries to the project
target_link_libraries( ${Unity_Plugin_Lib} ${XCB_LIBRARIES} ${Vulkan_LIBRARY})

qt5_use_modules(${Unity_Plugin_Lib} Widgets X11Extras)

target_compile_definitions(${Unity_Plugin_Lib} PRIVATE UNITY_BUILD)

############################################################
# Create an executable
############################################################

# Add an executable with the above sources
#file(GLOB_RECURSE QT_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/NativeUntyTriangleCode/QtAppSource/*.*)
#add_executable(${Qt_Binary} ${QT_SRC_FILES} ${COMMON_BASE_FILES} ${RESOURCE_FILES} ${QT_UNITY_IMPL})

#file(GLOB_RECURSE QT_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/triangle/*.*)
#add_executable(${Qt_Binary} ${QT_SRC_FILES} ${COMMON_BASE_FILES} ${RESOURCE_FILES})

file(GLOB_RECURSE QT_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/NativeUntyDynamicuniformbuffer/QtUnityDynamicUniformBufferImpl/*.*)
add_executable(${Qt_Binary} ${QT_SRC_FILES} ${COMMON_BASE_FILES} ${RESOURCE_FILES})

target_link_libraries(${Qt_Binary} ${XCB_LIBRARIES} ${Vulkan_LIBRARY})

# Define project properties
set_property(TARGET ${Qt_Binary} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Qt_Binary} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Qt_Binary} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Qt_Binary} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Qt_Binary} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/binaries)

qt5_use_modules(${Qt_Binary} Widgets X11Extras)

target_compile_definitions(${Qt_Binary} PRIVATE USE_QWINDOW)
