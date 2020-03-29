project(OSPGL)

cmake_minimum_required (VERSION 3.2.3)


OPTION(OSP_BUILD_SANATIZER          "Build with the address sanatizer" OFF)
OPTION(OSP_WARNINGS_ARE_ERRORS      "Build with the flag -Werror" OFF)
OPTION(OSP_ENABLE_COMPILER_WARNINGS "Build with the majority of compiler warnings enabled" OFF)

if(!MSVC)
	set (CMAKE_CXX_FLAGS "-g")
endif()


set (CMAKE_CXX_STANDARD 17)
# Useful for editing in VIM using YouCompleteMe
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE OSP_SOURCES "src/*.cpp")

file(GLOB_RECURSE IMGUI_SOURCES "dep/imgui/*.cpp")
file(GLOB BULLET_SOURCES "dep/bullet3/src/bt*.cpp")
set(GLAD_SOURCES "dep/glad/glad.c")
set(STB_SOURCES "dep/stb/stb_impl.cpp")
set(FASTNOISE_SOURCES "dep/FastNoise/FastNoise.cpp")

add_executable(OSPGL ${OSP_SOURCES} ${IMGUI_SOURCES} ${GLAD_SOURCES} ${BULLET_SOURCES} ${FASTNOISE_SOURCES} ${STB_SOURCES})

include_directories(dep)

add_subdirectory(dep/glfw)
target_link_libraries(OSPGL glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(OpenGL REQUIRED)
include_directories(OSPGL PUBLIC ${OPENGL_INCLUDE_DIR})

find_package(Threads)
target_link_libraries(OSPGL ${CMAKE_THREAD_LIBS_INIT})


#find_package(Lua REQUIRED)
#include_directories(OSPGL PUBLIC ${Lua_INCLUDE_DIR})

add_subdirectory(dep/LuaJIT-cmake)
add_subdirectory(dep/bullet3)
# Note: Double precision is defined on our custom bullet source, no need to define it here again
add_subdirectory(dep/fmt)
add_subdirectory(dep/assimp)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)

include_directories(OSPGL PUBLIC dep/assimp/include)   
include_directories(OSPGL PUBLIC dep/bullet3/src)



target_link_libraries(OSPGL fmt liblua-static assimp BulletSoftBody BulletDynamics BulletCollision LinearMath ${CMAKE_DL_LIBS})

if(!MSVC)
	target_link_libraries(OSPGL stdc++fs)
endif()