file(GLOB_RECURSE
	LANGULUS_ANYNESS_TEST_SOURCES 
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	*.cpp
)

add_executable(LangulusAnynessTest ${LANGULUS_ANYNESS_TEST_SOURCES})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	# There are some tests that involve safety when self = self is performed	
	# Also GCC is notorious with its uninitialized member false-positives		
    target_compile_options(LangulusAnynessTest
		PRIVATE -Wno-maybe-uninitialized
				-Wno-self-assign-overloaded
	)
endif()


target_link_libraries(LangulusAnynessTest
	PRIVATE		LangulusAnyness
				Catch2
)

add_test(
	NAME		LangulusAnynessTest 
	COMMAND		LangulusAnynessTest
)