cmake_minimum_required (VERSION 3.2.3)

# Define basic compiler and CMake configuration

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (OSPGL_STACKTRACES ON)

##################################################################################
# OSPGL - The game engine
##################################################################################
project(OSPGL)


# Define source files for OSP and non-cmake dependencies

file(GLOB_RECURSE OSP_SOURCES "src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCES "dep/imgui/*.cpp")
set(GLAD_SOURCES "dep/glad/glad.c")
set(STB_SOURCES "dep/stb/stb_impl.cpp")
set(FASTNOISEC_SOURCES "dep/FastNoiseC/FastNoise.c")
set(NANOVG_SOURCES "dep/nanovg/nanovg_ext.cpp")
set(TINY_GLTF_SOURCES "dep/tiny_gltf/tiny_gltf.cpp")
set(BACKWARD_SOURCES "dep/tiny_gltf/tiny_gltf.cpp")
set(MINIAUDIO_SOURCES "dep/miniaudio/miniaudio.c")

# Create the OSPGL executable

add_executable(OSPGL ${OSP_SOURCES} ${IMGUI_SOURCES} ${GLAD_SOURCES} 
	${FASTNOISEC_SOURCES} ${STB_SOURCES} ${NANOVG_SOURCES} ${TINY_GLTF_SOURCES}
	${BACKWARD_SOURCES} ${MINIAUDIO_SOURCES})

# Name it ospgl (lowercase)
set_target_properties(OSPGL PROPERTIES OUTPUT_NAME "ospgl")

# Set compiler options for OSPGL

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
	target_compile_options(OSPGL PUBLIC /bigobj)
else()
	target_compile_options(OSPGL PUBLIC -g -Werror -O0)
	set(EXTRA_LINK "stdc++fs")
	if(OSPGL_STACKTRACES)
		add_definitions(-DBACKWARD_HAS_BFD=1)
		set(STACKTRACE_LINK "bfd")
	endif()

	link_libraries(${CMAKE_DL_LIBS})
endif()

include_directories(dep)
include_directories(OSPGL PUBLIC dep/bullet3/src)
include_directories(OSPGL PUBLIC src)
include_directories(OSPGL PUBLIC pkg)

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

find_package(Threads REQUIRED)
target_link_libraries(OSPGL ${CMAKE_THREAD_LIBS_INIT})

# LuaJIT 

add_subdirectory(dep/LuaJIT-cmake)

# Bullet3
add_subdirectory(dep/bullet3)
# Make sure we use double precision, and it's enabled for
# the bullet compilation
set(USE_DOUBLE_PRECISION ON CACHE BOOL "" FORCE)
ADD_DEFINITIONS( -DBT_USE_DOUBLE_PRECISION)
set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE) #< Bullet3 is incomplete and uses only floats

# fmt

add_subdirectory(dep/fmt)

# FreeType
find_package(Freetype)
if(Freetype_FOUND)
	target_link_Libraries(OSPGL ${FREETYPE_LIBRARIES})
	include_directories(OSPGL ${FREETYPE_INCLUDE_DIRS})
	add_compile_definitions(WITH_FREETYPE)
else()
	message(WARNING "Could not find FreeType, text quality will be worse")
endif()

# Link libraries

target_link_libraries(OSPGL fmt liblua-static BulletSoftBody BulletDynamics
		BulletCollision LinearMath ${CMAKE_DL_LIBS} ${EXTRA_LINK} ${STACKTRACE_LINK})

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
	target_compile_options(ospm PUBLIC -g -O0)
endif()

add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

find_package(OpenSSL REQUIRED)
message(STATUS "OpenSSL Version: " ${OPENSSL_VERSION})
message(STATUS "OpenSSL Include: " ${OPENSSL_INCLUDE_DIR})

include_directories(dep)
include_directories(pkg)
include_directories(${OPENSSL_INCLUDE_DIR})

target_link_libraries(ospm ${OPENSSL_LIBRARIES} ${CMAKE_DL_LIBS} ${EXTRA_LINK})
