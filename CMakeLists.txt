cmake_minimum_required(VERSION 3.28)
project(LangulusAnyness
    VERSION         1.0.0
    DESCRIPTION     "Langulus container library"
    HOMEPAGE_URL    https://langulus.com
)

# Check if this project is built as standalone, or a part of something else 
if(PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

    include(LangulusUtilities.cmake)

    # Add Langulus::Core/Logger/RTTI/Fractalloc/SIMD libraries              
    fetch_langulus_module(Core   GIT_TAG 8c77fc2752d1ab827c8f90584e83ccb5d916a946)
    fetch_langulus_module(Logger GIT_TAG f32c2b76c2bcc32b6f36f1edda6c11378cb85b1d)
    fetch_langulus_module(RTTI   GIT_TAG de87db12baccd7763f7188b2c2eec5ae620b6d04)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG c3dbd808c15f422ff3eba22c62b0114d3c6d0791)
    endif()
    fetch_langulus_module(SIMD   GIT_TAG d96d5e19d64717f2d4e530606660f76b6cc1357e)
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

target_include_directories(LangulusAnyness PUBLIC include
    $<TARGET_PROPERTY:LangulusLogger,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:LangulusRTTI,INTERFACE_INCLUDE_DIRECTORIES>
    $<$<BOOL:${LANGULUS_FEATURE_MANAGED_MEMORY}>:$<TARGET_PROPERTY:LangulusFractalloc,INTERFACE_INCLUDE_DIRECTORIES>>
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