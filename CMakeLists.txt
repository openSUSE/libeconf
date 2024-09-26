cmake_minimum_required(VERSION 3.12)

# Ensure built-in policies from CMake are used, (e.g. improved policies for macOS)
cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})

project(libeconf VERSION 0.7.4
                 DESCRIPTION "Enhanced config file parser, which merges config files placed in several locations into one."
                 LANGUAGES C
                 )

# Set up GNU conventions and standard FHS paths
include(GNUInstallDirs)

# Activate CMake package configuration helpers
include(CMakePackageConfigHelpers)

# Ensure _GNU_SOURCE is available
include(CheckSymbolExists)

# check if _GNU_SOURCE is available

if (NOT _GNU_SOURCE)
    check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

    if (NOT _GNU_SOURCE)
        unset(_GNU_SOURCE CACHE)
        check_symbol_exists(_GNU_SOURCE "features.h" _GNU_SOURCE)
    endif ()
endif ()

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif ()


# set the minimum C standard
set(CMAKE_C_STANDARD 11)

# Build Types
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Sanitize RelWithDebInfoStrict"
    FORCE
    )

# AddressSanitize
set(CMAKE_C_FLAGS_SANITIZE
    "-O2 -g -Wall -fsanitize=address,undefined"
    CACHE STRING "Flags used by the C compiler during Sanitizer builds."
    FORCE
    )

set(CMAKE_LINK_FLAGS_SANITIZE
    "-fsanitize=address,undefined"
    CACHE STRING "Flags used by the linker during Sanitizer builds."
    FORCE
    )

# RelWithDebInfoStrict
set(CMAKE_C_FLAGS_RELWITHDEBINFOSTRICT
    "-O2 -g -Werror -W -Wall -DXTSTRINGDEFINES -D_FORTIFY_SOURCE=2 -fstack-protector-strong -funwind-tables -fasynchronous-unwind-tables -fstack-clash-protection -Werror=return-type -flto=8 -Wbad-function-cast -Wcast-align -Wcast-qual -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wshadow -Wstrict-prototypes -Wundef"
    CACHE STRING "Flags used by the C compiler during strict RelWithDebInfo builds."
    FORCE
    )

add_subdirectory(lib)
add_subdirectory(example)
add_subdirectory(util)
add_subdirectory(doc)

# Enable testing
include(CTest)
enable_testing()
add_subdirectory(tests)
