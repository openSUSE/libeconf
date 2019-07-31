cmake_minimum_required(VERSION 2.8.5)
cmake_policy(VERSION ${CMAKE_VERSION})

set(VERSION 0.1.0)
if(${CMAKE_VERSION} VERSION_LESS "3.0.0")
  project(libeconf C)
  set(PROJECT_VERSION ${VERSION})
else()
  project(libeconf LANGUAGES C VERSION ${VERSION})
endif()

include(GNUInstallDirs)

add_compile_options(-std=gnu11)

# Create the library
add_library(econf SHARED src/libeconf.c src/getfilecontents.c src/mergefiles.c src/helpers.c src/keyfile.c)
set_target_properties(econf PROPERTIES
  VERSION ${PROJECT_VERSION}
  PUBLIC_HEADER include/libeconf.h
)

# Install the library
configure_file(lib/libeconf.pc.cmake libeconf.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libeconf.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)
install(TARGETS econf
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Create the binary/executable
add_executable(example EXCLUDE_FROM_ALL example/example.c)
target_include_directories(example PRIVATE include)
target_link_libraries(example PRIVATE econf -lm)
target_link_libraries(example PRIVATE -Werror -Wall -O2)


# Use these targets for debugging

# Debugging with valgrind / gdb
add_executable(valgrind EXCLUDE_FROM_ALL example/example.c)
target_include_directories(valgrind PRIVATE include)
target_link_libraries(valgrind PRIVATE econf -lm)
target_link_libraries(valgrind PRIVATE -Werror -Wall -g -O0 -fno-omit-frame-pointer)

# Debugging with Address-Sanitizer
add_custom_target(Asanitizer)
add_dependencies(Asanitizer econf-static Asan)

add_library(econf-static STATIC EXCLUDE_FROM_ALL src/libeconf.c src/getfilecontents.c src/mergefiles.c src/helpers.c src/keyfile.c)
target_link_libraries(econf-static PRIVATE -Werror -Wall -g -O0 -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer)
set_target_properties(econf-static PROPERTIES
  VERSION ${PROJECT_VERSION}
  PUBLIC_HEADER include/libeconf.h
)

add_executable(Asan EXCLUDE_FROM_ALL example/example.c)
target_include_directories(Asan PRIVATE include)
target_link_libraries(Asan PRIVATE econf -lm)
target_link_libraries(Asan PRIVATE -Werror -Wall -g -O0 -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer)
target_compile_options(Asan PRIVATE -fsanitize=address -fsanitize=undefined)
