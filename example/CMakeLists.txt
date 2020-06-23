# Create the binary/executable
add_executable(example EXCLUDE_FROM_ALL example.c)
target_link_libraries(example PRIVATE econf)
