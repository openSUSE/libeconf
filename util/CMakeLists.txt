# Create the binary/executable
add_executable(econftool econftool.c)
target_link_libraries(econftool PRIVATE econf)

install(TARGETS econftool
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
