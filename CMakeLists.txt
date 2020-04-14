cmake_minimum_required (VERSION 3.2.3)

# Define basic compiler and CMake configuration

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)


##################################################################################
# OSPGL - The game engine
##################################################################################
project(OSPGL)



# Define source files for OSP and non-cmake dependencies

file(GLOB_RECURSE OSP_SOURCES "src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCES "dep/imgui/*.cpp")
set(GLAD_SOURCES "dep/glad/glad.c")
set(STB_SOURCES "dep/stb/stb_impl.cpp")
set(FASTNOISE_SOURCES "dep/FastNoise/FastNoise.cpp")
set(NANOVG_SOURCES "dep/nanovg/nanovg.c")


# Create the OSPGL executable

add_executable(OSPGL ${OSP_SOURCES} ${IMGUI_SOURCES} ${GLAD_SOURCES} ${FASTNOISE_SOURCES} ${STB_SOURCES} ${NANOVG_SOURCES})

# Name it ospgl (lowercase)
set_target_properties(OSPGL PROPERTIES OUTPUT_NAME "ospgl")

# Set compiler options for OSPGL

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
	target_compile_options(OSPGL PUBLIC /bigobj)
else()
	target_compile_options(OSPGL PUBLIC -g -Werror)
endif()

include_directories(dep)
include_directories(OSPGL PUBLIC dep/ospgl-assimp/include)   
include_directories(OSPGL PUBLIC dep/bullet3/src)
include_directories(OSPGL PUBLIC src)

# Include CMake dependencies

# GLFW
add_subdirectory(dep/glfw)
target_link_libraries(OSPGL glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# OpenGL

find_package(OpenGL REQUIRED)
include_directories(OSPGL PUBLIC ${OPENGL_INCLUDE_DIR})

# Threads

find_package(Threads)
target_link_libraries(OSPGL ${CMAKE_THREAD_LIBS_INIT})

# LuaJIT 

add_subdirectory(dep/LuaJIT-cmake)

# Bullet3
add_subdirectory(dep/bullet3)
# Note: Double precision is defined on our custom bullet source, no need to define it here again

# fmt

add_subdirectory(dep/fmt)

# Assimp

add_subdirectory(dep/ospgl-assimp)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)

# Link libraries

target_link_libraries(OSPGL fmt liblua-static assimp BulletSoftBody BulletDynamics BulletCollision LinearMath ${CMAKE_DL_LIBS})

##################################################################################
# ospm - The package manager for OSPGL (Open Space Program Manager)
##################################################################################

project(ospm)

file(GLOB_RECURSE OSPM_SOURCES "ospm_src/*.cpp")
set(MINIZ_SOURCES "ospm_src/dep/miniz.c")

add_executable(ospm ${OSPM_SOURCES} ${MINIZ_SOURCES})

if(MSVC)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
else()
    target_compile_options(OSPGL PUBLIC -g -Werror)
endif()

add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

find_package(OpenSSL REQUIRED)
message(STATUS "OpenSSL Version: " ${OPENSSL_VERSION})
message(STATUS "OpenSSL Include: " ${OPENSSL_INCLUDE_DIR})

include_directories(dep)
include_directories(${OPENSSL_INCLUDE_DIR})

target_link_libraries(ospm ${OPENSSL_LIBRARIES} ${CMAKE_DL_LIBS})
