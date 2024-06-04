# Create the library
set(econf_SRCS libeconf.c
               libeconf_ext.c
               getfilecontents.c
               mergefiles.c
               helpers.c
               keyfile.c
               econf_error.c
               get_value_def.c
	       readconfig.c
               )

set(econf_HDRS defines.h
               getfilecontents.h
               mergefiles.h
               helpers.h
               keyfile.h
	       readconfig.h
               )

add_library(econf SHARED ${econf_SRCS} ${econf_HDRS}
               "${PROJECT_SOURCE_DIR}/include/libeconf.h" "${PROJECT_SOURCE_DIR}/include/libeconf_ext.h")

target_include_directories(econf PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(econf PROPERTIES
  SOVERSION ${PROJECT_VERSION_MAJOR}
  VERSION ${PROJECT_VERSION}
  COMPILE_FLAGS "-D_REENTRANT=1"
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
  PUBLIC_HEADER "${PROJECT_SOURCE_DIR}/include/libeconf.h;${PROJECT_SOURCE_DIR}/include/libeconf_ext.h"
  LINK_FLAGS "-Wl,--no-undefined -Wl,--version-script,\"${PROJECT_SOURCE_DIR}/lib/libeconf.map\""
)

# Install the library
configure_file(${PROJECT_SOURCE_DIR}/cmake/libeconf.pc.in ${CMAKE_BINARY_DIR}/libeconf.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libeconf.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(TARGETS econf
  EXPORT libeconfTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Generate and install the exports
export(EXPORT libeconfTargets
  FILE ${CMAKE_BINARY_DIR}/libeconfTargets.cmake
)
install(EXPORT libeconfTargets
  FILE libeconfTargets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libeconf
)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/libeconfConfig.cmake.in
  ${CMAKE_BINARY_DIR}/libeconfConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libeconf
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
install(FILES
  ${CMAKE_BINARY_DIR}/libeconfConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libeconf
)
write_basic_package_version_file(
  ${CMAKE_BINARY_DIR}/libeconfConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion
)
install(FILES
  ${CMAKE_BINARY_DIR}/libeconfConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libeconf
)
