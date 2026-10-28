cmake_minimum_required(VERSION 3.16)
# Set the SDK floor before compiler detection, including melonDS and generated ASM.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    if(DEFINED ENV{MACOSX_DEPLOYMENT_TARGET})
        set(CMAKE_OSX_DEPLOYMENT_TARGET "$ENV{MACOSX_DEPLOYMENT_TARGET}" CACHE STRING "Minimum macOS version")
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    endif()
endif()
project(ssrecomp LANGUAGES C CXX)
set(SSR_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gen" CACHE PATH "Generated game sources")
option(SSR_UWP "Build native SoulSilver frontend for Xbox UWP x64" OFF)
set(SSR_SDL2_WINRT_SOURCE_DIR "" CACHE PATH "SDL2 checkout containing its WinRT non-XAML main function")
if(SSR_UWP)
    if(NOT WIN32)
        message(FATAL_ERROR "SSR_UWP requires a WindowsStore UWP toolchain")
    endif()
    add_compile_definitions(_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
endif()

if(SSR_UWP)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# melonDS core is the hardware runtime; its own frontend, JIT and GL renderer are not used.
set(BUILD_QT_SDL OFF CACHE BOOL "" FORCE)
set(ENABLE_OGLRENDERER OFF CACHE BOOL "" FORCE)
set(ENABLE_JIT OFF CACHE BOOL "" FORCE)
set(ENABLE_GDBSTUB OFF CACHE BOOL "" FORCE)
set(ENABLE_LTO_RELEASE OFF CACHE BOOL "" FORCE)
add_subdirectory(melonDS EXCLUDE_FROM_ALL)

option(SSR_MACOS_SOURCE_DEPS "Build SDL2 and libpng for the selected macOS target" ${APPLE})
if(SSR_UWP)
    # Match the standalone SDL2 WinRT workflow that successfully built on MSVC.
    # Using an independent static build in this project avoids a broken or
    # architecture-mismatched vcpkg desktop SDL import.
    if(NOT EXISTS "${SSR_SDL2_WINRT_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "SSR_UWP needs -DSSR_SDL2_WINRT_SOURCE_DIR=<SDL2 source tree>")
    endif()
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
    set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
    set(SDL_LIBSAMPLERATE OFF CACHE BOOL "" FORCE)
    # SDL2 WinRT uses Microsoft C++/CX (/ZW). MSVC rejects /ZW with
    # /std:c++20. The game and melonDS still require C++20, but SDL2 must
    # inherit the compiler's default language mode, as in its passing CI.
    set(SSR_GAME_CXX_STANDARD "${CMAKE_CXX_STANDARD}")
    unset(CMAKE_CXX_STANDARD)
    add_subdirectory("${SSR_SDL2_WINRT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/uwp-sdl2" EXCLUDE_FROM_ALL)
    set(CMAKE_CXX_STANDARD "${SSR_GAME_CXX_STANDARD}")
elseif(APPLE AND NOT ANDROID AND SSR_MACOS_SOURCE_DEPS)
    include(cmake/MacDependencies.cmake)
else()
    find_package(SDL2 REQUIRED)
    if(NOT ANDROID)
        find_package(PNG REQUIRED)
    endif()
endif()

if(NOT MSVC)
    enable_language(ASM)
endif()

# Recompiled game code, produced by tools/recomp.py.
include("${SSR_GENERATED_DIR}/sources.cmake")
if(SSR_UWP)
    foreach(source IN LISTS RC_GENERATED_SOURCES)
        if(source MATCHES "[.]S$")
            message(FATAL_ERROR "Generate your private game code with tools/recomp.py --uwp first")
        endif()
    endforeach()
endif()
add_library(gamecode OBJECT ${RC_GENERATED_SOURCES})
target_include_directories(gamecode PRIVATE melonDS/src runtime "${SSR_GENERATED_DIR}")
# Generated code is huge and uniform; -O1 keeps build times reasonable.
if(MSVC)
    target_compile_options(gamecode PRIVATE /O1 /bigobj /GR- /GS-)
else()
    target_compile_options(gamecode PRIVATE -O1 -g0 -fno-exceptions -fno-rtti "$<$<COMPILE_LANGUAGE:CXX>:-fno-stack-protector>"
        "$<$<COMPILE_LANGUAGE:ASM>:-Wa,-I${SSR_GENERATED_DIR}>")
endif()

set(SSRECOMP_SOURCES
    frontend/main.cpp
    frontend/platform.cpp
    frontend/uwp/storage.cpp
    frontend/uwp/gamepad.cpp
    frontend/uwp/gamepad_reader.cpp
    runtime/recomp_rt.cpp
    runtime/recomp_mem.cpp
    "${SSR_GENERATED_DIR}/apps_table.cpp"
    $<TARGET_OBJECTS:gamecode>)
if(ANDROID)
    # On Android the game is the library libmain.so that SDL's Java activity loads
    # (tools/android/apk.py packs it into an APK). SDL2 is one built for Android
    # (CMAKE_PREFIX_PATH); no libpng: screenshots are a desktop test aid.
    add_library(ssrecomp SHARED ${SSRECOMP_SOURCES})
    set_target_properties(ssrecomp PROPERTIES OUTPUT_NAME main)
    target_include_directories(ssrecomp PRIVATE melonDS/src frontend runtime)
    # The app's version, for its check for a newer release: the newest one in CHANGELOG.md.
    set(SSR_VERSION "")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS CHANGELOG.md)
        file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md" SSR_RELEASES REGEX "^## v[0-9]")
        if(SSR_RELEASES)
            list(GET SSR_RELEASES 0 SSR_VERSION)
            string(REGEX REPLACE "^## v([0-9.]+).*" "\\1" SSR_VERSION "${SSR_VERSION}")
        endif()
    endif()
    target_compile_definitions(ssrecomp PRIVATE SSR_NO_PNG SSR_VERSION="${SSR_VERSION}")
    target_link_libraries(ssrecomp PRIVATE core SDL2::SDL2 android log)
else()
    if(SSR_UWP)
        add_executable(ssrecomp WIN32 ${SSRECOMP_SOURCES})
        # SDL2's Xbox/WinRT entry point translates CoreApplication lifecycle
        # events into the existing SDL_main(argc, argv) game frontend.
        set(SSR_WINRT_MAIN "${SSR_SDL2_WINRT_SOURCE_DIR}/src/main/winrt/SDL_winrt_main_NonXAML.cpp")
        if(NOT EXISTS "${SSR_WINRT_MAIN}")
            message(FATAL_ERROR "Provide -DSSR_SDL2_WINRT_SOURCE_DIR=<SDL2 checkout> for the UWP main wrapper")
        endif()
        target_sources(ssrecomp PRIVATE "${SSR_WINRT_MAIN}")
        # SDL2 WinRT also ships cursor resources required by SDL_ShowCursor.
        set(SSR_WINRT_RESOURCES "${SSR_SDL2_WINRT_SOURCE_DIR}/src/main/winrt/SDL2-WinRTResources.rc")
        if(EXISTS "${SSR_WINRT_RESOURCES}")
            target_sources(ssrecomp PRIVATE "${SSR_WINRT_RESOURCES}")
        endif()
        set_source_files_properties("${SSR_WINRT_MAIN}" PROPERTIES COMPILE_OPTIONS "/ZW;/std:c++14")
    else()
        add_executable(ssrecomp ${SSRECOMP_SOURCES})
    endif()
    target_include_directories(ssrecomp PRIVATE melonDS/src frontend runtime ${SDL2_INCLUDE_DIRS})
    if(SSR_UWP)
        target_include_directories(ssrecomp PRIVATE "${SSR_SDL2_WINRT_SOURCE_DIR}/include")
        # Prevent C++/CX static WinRT objects from being discarded by LINK.
        target_link_options(ssrecomp PRIVATE "/WHOLEARCHIVE:$<TARGET_FILE:SDL2-static>")
        target_compile_options(ssrecomp PRIVATE /bigobj)
        set_target_properties(ssrecomp PROPERTIES CXX_STANDARD 20)
    endif()
    find_package(Threads REQUIRED)
    if(SSR_UWP)
        target_link_libraries(ssrecomp PRIVATE SDL2-static)
    elseif(TARGET SDL2::SDL2)
        if(TARGET SDL2::SDL2main)
            target_link_libraries(ssrecomp PRIVATE SDL2::SDL2main)
        endif()
        target_link_libraries(ssrecomp PRIVATE SDL2::SDL2)
    else()
        target_link_libraries(ssrecomp PRIVATE ${SDL2_LIBRARIES})
    endif()
    if(SSR_UWP)
        target_compile_definitions(ssrecomp PRIVATE SSR_NO_PNG SSR_UWP=1 NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS _SILENCE_CXX20_U8PATH_DEPRECATION_WARNING)
        target_link_libraries(ssrecomp PRIVATE core Threads::Threads WindowsApp.lib d3d11.lib dxgi.lib)
    else()
        target_link_libraries(ssrecomp PRIVATE core PNG::PNG Threads::Threads)
    endif()
endif()
