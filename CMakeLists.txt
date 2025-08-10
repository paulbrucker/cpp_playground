cmake_minimum_required(VERSION 3.15)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

include(Dependencies.cmake)
include(ProjectOptions.cmake)

# Enable all warnings and treat warnings as errors.
# Warning on signed/unsigned comparison is disabled to allow comparison of
# size_t and int, which is common in C++ code.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-sign-compare)
    # Enable sanitizers in Debug builds
    if(ENABLE_SANITIZERS)
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX /wd4018)
endif()


if(BUILD_DOXYGEN)
    add_subdirectory(docs)
endif()

# Add subdirectories with code
add_subdirectory(src)
add_subdirectory(app)

# Clang-Tidy integration to enforce conformity to the C++ Core Guidelines
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()