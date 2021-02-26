# Create the binary/executable
add_executable(econftool econftool.c)
target_link_libraries(econftool PRIVATE econf)
