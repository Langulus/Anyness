cmake_minimum_required(VERSION 3.28)
project(LangulusAnyness
    VERSION         1.0.0
    DESCRIPTION     "Langulus container library"
    HOMEPAGE_URL    https://langulus.com
)

# Check if this project is built as standalone, or a part of something else		
if (PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
    include(LangulusUtilities.cmake)

    fetch_langulus_module(RTTI GIT_TAG c7620c931c2cf26ac345a9155b569631f5223896)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG f3f7d6c2a3b080ae3ecb39ea73791d057510876c)
    endif()
    fetch_langulus_module(SIMD GIT_TAG a3dce7f6405bf64e576b884de9a134855292573c)
endif()

file(GLOB_RECURSE
	LANGULUS_ANYNESS_SOURCES 
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	source/*.cpp
)

# Build and install Anyness library												
add_langulus_library(LangulusAnyness
    $<TARGET_OBJECTS:LangulusLogger>
    $<TARGET_OBJECTS:LangulusRTTI>
    $<$<BOOL:${LANGULUS_FEATURE_MANAGED_MEMORY}>:$<TARGET_OBJECTS:LangulusFractalloc>>
    ${LANGULUS_ANYNESS_SOURCES}
)

target_include_directories(LangulusAnyness
	PUBLIC		$<TARGET_PROPERTY:LangulusLogger,INTERFACE_INCLUDE_DIRECTORIES>
				$<TARGET_PROPERTY:LangulusRTTI,INTERFACE_INCLUDE_DIRECTORIES>
				$<$<BOOL:${LANGULUS_FEATURE_MANAGED_MEMORY}>:$<TARGET_PROPERTY:LangulusFractalloc,INTERFACE_INCLUDE_DIRECTORIES>>
				include
)

target_link_libraries(LangulusAnyness
    PUBLIC      LangulusCore
                fmt
)

target_compile_definitions(LangulusAnyness
    PRIVATE     LANGULUS_EXPORT_ALL
)

if(LANGULUS_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()