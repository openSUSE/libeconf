# Create the binary/executable
add_executable(example EXCLUDE_FROM_ALL example.c)
target_compile_options(example PRIVATE -DDEXAMPLEDIR=\"${PROJECT_SOURCE_DIR}/example/\")
target_link_libraries(example PRIVATE econf)
