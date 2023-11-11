file(GLOB_RECURSE
	LANGULUS_ANYNESS_TEST_SOURCES 
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	*.cpp *.hpp
)

add_executable(LangulusAnynessTest ${LANGULUS_ANYNESS_TEST_SOURCES})

target_link_libraries(LangulusAnynessTest
	PRIVATE		LangulusAnyness
				Catch2
)

add_test(
	NAME		LangulusAnynessTest 
	COMMAND		LangulusAnynessTest
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# There are some tests that involve safety when self = self is performed	
    target_compile_options(LangulusAnynessTest PRIVATE -Wno-self-assign-overloaded)
endif()

langulus_copy_dlls(LangulusAnynessTest ON LangulusAnynessTest)
