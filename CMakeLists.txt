cmake_minimum_required(VERSION 3.25)
project(longhorn VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")

# Use mold linker if available
find_program(MOLD_LINKER mold)
if(MOLD_LINKER)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    message(STATUS "Using mold linker")
endif()

# Options
option(BUILD_TESTS "Build test suite" ON)
option(USE_SYSTEM_SDL "Use system SDL3 instead of submodules" OFF)

# --- OpenGL ---
find_package(OpenGL REQUIRED)

# --- Embed GLSL shaders as C string arrays ---
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

function(embed_shader SHADER_NAME SHADER_STAGE)
    set(GLSL_FILE ${SHADER_DIR}/${SHADER_NAME}.${SHADER_STAGE}.glsl)
    set(C_FILE ${SHADER_OUT_DIR}/${SHADER_NAME}_${SHADER_STAGE}_glsl.c)
    set(ARRAY_NAME "${SHADER_NAME}_${SHADER_STAGE}_glsl")

    add_custom_command(
        OUTPUT ${C_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DINPUT_FILE=${GLSL_FILE}
            -DOUTPUT_FILE=${C_FILE}
            -DARRAY_NAME=${ARRAY_NAME}
            -P ${CMAKE_SOURCE_DIR}/cmake/embed_text.cmake
        DEPENDS ${GLSL_FILE} ${CMAKE_SOURCE_DIR}/cmake/embed_text.cmake
        COMMENT "Embedding ${SHADER_NAME}.${SHADER_STAGE}.glsl"
    )

    set(SHADER_C_FILES ${SHADER_C_FILES} ${C_FILE} PARENT_SCOPE)
endfunction()

embed_shader(glass vert)
embed_shader(glass frag)

# --- SDL3 dependency ---
if(USE_SYSTEM_SDL)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3 REQUIRED sdl3)
    pkg_check_modules(SDL3_TTF REQUIRED sdl3-ttf)
else()
    message(STATUS "Building SDL3 from submodules")

    set(SDL_SHARED ON CACHE BOOL "Build SDL3 shared")
    set(SDL_STATIC OFF CACHE BOOL "No static SDL3")
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "No SDL3 tests")
    add_subdirectory(submodules/SDL)

    set(SDL3TTF_INSTALL OFF CACHE BOOL "")
    set(SDL3TTF_VENDORED ON CACHE BOOL "")
    add_subdirectory(submodules/SDL_ttf)
endif()

# --- Longhorn library ---
add_library(longhorn_lib STATIC
    src/app.cpp
    src/renderer.cpp
    src/input.cpp
    src/filter.cpp
    src/geometry.cpp
    src/glass.cpp
    src/gl_funcs.cpp
    src/gl_text.cpp
    ${SHADER_C_FILES}
)

target_include_directories(longhorn_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

if(USE_SYSTEM_SDL)
    target_include_directories(longhorn_lib PUBLIC ${SDL3_INCLUDE_DIRS} ${SDL3_TTF_INCLUDE_DIRS})
    target_link_libraries(longhorn_lib PUBLIC ${SDL3_LIBRARIES} ${SDL3_TTF_LIBRARIES} OpenGL::GL)
else()
    target_link_libraries(longhorn_lib PUBLIC SDL3::SDL3 SDL3_ttf::SDL3_ttf OpenGL::GL)
endif()

# --- Main executable ---
add_executable(longhorn src/main.cpp)
target_link_libraries(longhorn PRIVATE longhorn_lib)

# --- Install ---
include(GNUInstallDirs)
install(TARGETS longhorn DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY shaders/ DESTINATION ${CMAKE_INSTALL_DATADIR}/longhorn/shaders
    FILES_MATCHING PATTERN "*.glsl")
install(FILES doc/longhorn.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES longhorn.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# --- Tests ---
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(submodules/doctest)

    add_executable(longhorn_tests
        tests/main.cpp
        tests/test_filter.cpp
        tests/test_geometry.cpp
    )
    target_link_libraries(longhorn_tests PRIVATE longhorn_lib doctest::doctest)
    target_include_directories(longhorn_tests PRIVATE ${CMAKE_SOURCE_DIR}/submodules/doctest/doctest)

    add_test(NAME LonghornTests COMMAND longhorn_tests)

    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS longhorn_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running tests..."
    )
endif()
