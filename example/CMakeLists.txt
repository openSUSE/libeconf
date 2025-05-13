# Create the binary/executable
add_executable(example example.c)
target_compile_options(example PRIVATE -DEXAMPLEDIR=\"${PROJECT_SOURCE_DIR}/example/\")
target_link_libraries(example PRIVATE econf)
