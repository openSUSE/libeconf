cmake_minimum_required(VERSION 3.13.0)
project(libeconf VERSION 0.1.0 DESCRIPTION "TODO")
include(GNUInstallDirs)

# Create the library
add_library(econf SHARED src/libeconf.c)
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
target_link_libraries(example PRIVATE econf)
# Enable for debugging
# target_link_libraries(example PRIVATE -g3 -fsanitize=address -fno-omit-frame-pointer)
