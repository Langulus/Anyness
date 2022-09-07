project(Langulus_Test_Anyness)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)

# Configure Catch2 library
message(STATUS "Configuring external project: Catch2...")
download_project(
	PROJ                Catch2
	GIT_REPOSITORY      https://github.com/catchorg/Catch2.git
	GIT_TAG             v2.x
	UPDATE_DISCONNECTED 1
)

add_compile_definitions(LANGULUS_ENABLE_TESTING)
add_compile_definitions(LANGULUS_ENABLE_ASSERTION_LEVEL=1000)

add_subdirectory(${Catch2_SOURCE_DIR} ${Catch2_BINARY_DIR})

add_executable(Test.Anyness ${SOURCE_FILES})

target_link_libraries(Test.Anyness PRIVATE Langulus.Anyness Catch2)

add_test(NAME Test.Anyness COMMAND Test.Anyness)
