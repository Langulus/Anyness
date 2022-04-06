project(TestAnyness)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)
add_executable(TestAnyness ${SOURCE_FILES})
target_link_libraries(TestAnyness PRIVATE Anyness)
