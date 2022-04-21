project(TestAnyness)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)

# Configure Catch2 library
message(STATUS "Configuring external project: Catch2...")
#set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
#set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
#set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "" FORCE)

download_project(
	PROJ                Catch2
	GIT_REPOSITORY      https://github.com/catchorg/Catch2.git
	GIT_TAG             v2.x
	GIT_PROGRESS        TRUE
	UPDATE_DISCONNECTED 1
)
add_subdirectory(${Catch2_SOURCE_DIR} ${Catch2_BINARY_DIR})

add_executable(TestAnyness ${SOURCE_FILES})
target_link_libraries(TestAnyness PRIVATE Anyness Catch2)
