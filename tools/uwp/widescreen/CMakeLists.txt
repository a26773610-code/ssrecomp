cmake_minimum_required(VERSION 3.22)
project(PokemonDSXbox LANGUAGES C CXX)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Use WindowsStore x64 with the ClangCL toolset")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clang:-fwrapv /clang:-Wno-c++11-narrowing /Zc:char8_t- /bigobj")
add_compile_definitions(WINAPI_FAMILY=WINAPI_FAMILY_APP _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
set(BUILD_QT_SDL OFF CACHE BOOL "" FORCE)
set(ENABLE_JIT OFF CACHE BOOL "" FORCE)
set(ENABLE_OGLRENDERER OFF CACHE BOOL "" FORCE)
set(ENABLE_GDBSTUB OFF CACHE BOOL "" FORCE)
set(ENABLE_LTO_RELEASE OFF CACHE BOOL "" FORCE)
set(SSR_PORTABLE_X86 ON CACHE BOOL "" FORCE)
add_subdirectory("${SWITCH_SOURCE}/melonDS" melonDS)
set_property(TARGET core PROPERTY CXX_STANDARD 20)
find_package(SDL2 CONFIG REQUIRED)
set(GEN "${SWITCH_SOURCE}/build-watch")
get_filename_component(ADAPTER "${CMAKE_CURRENT_SOURCE_DIR}/../../../frontend/uwp" ABSOLUTE)
add_executable(pokemon-ds-wide WIN32
    "${SWITCH_SOURCE}/frontend/main.cpp" "${SWITCH_SOURCE}/frontend/platform.cpp"
    "${SWITCH_SOURCE}/runtime/recomp_rt.cpp" "${SWITCH_SOURCE}/runtime/recomp_mem.cpp"
    "${GEN}/rc_tables.cpp" "${GEN}/rc_portable.cpp" "${GEN}/apps_table.cpp"
    "${ADAPTER}/gamepad_reader.cpp" "${ADAPTER}/storage.cpp"
    xbox_start.cpp winmain.cpp
    "${SDL_SOURCE}/src/main/winrt/SDL2-WinRTResources.rc")
target_include_directories(pokemon-ds-wide PRIVATE
    "${SWITCH_SOURCE}/melonDS/src" "${SWITCH_SOURCE}/frontend" "${SWITCH_SOURCE}/runtime"
    "${ADAPTER}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(pokemon-ds-wide PRIVATE SSR_UWP=1 SSR_NO_PNG=1)
target_link_libraries(pokemon-ds-wide PRIVATE core SDL2::SDL2-static WindowsApp runtimeobject)
target_link_options(pokemon-ds-wide PRIVATE /APPCONTAINER /SUBSYSTEM:WINDOWS /STACK:8388608)
# Packaging is handled explicitly by MakeAppx, with the reviewed manifest.
set_target_properties(pokemon-ds-wide PROPERTIES VS_GLOBAL_AppxPackage false VS_GLOBAL_GenerateAppxPackageOnBuild false VS_GLOBAL_AppxPackageSigningEnabled false)
