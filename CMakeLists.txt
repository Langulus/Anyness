cmake_minimum_required(VERSION 3.24)
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
    fetch_langulus_module(Core   GIT_TAG c11832957380709e316ffaf737d47c4578ed8caf)
    fetch_langulus_module(Logger GIT_TAG e137aac46f3d682a06e8d0b1a160748fcbca21a4)
    fetch_langulus_module(RTTI   GIT_TAG 74d1a5fd0f5f6c23fd23c7921e1ba1c5df8c94c3)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG 9c46cbf6e10585b98f25b3d97ed35c1be7b14e45)
    endif()
    fetch_langulus_module(SIMD   GIT_TAG f70c0a00839c23024acc045d0ffb5eda1e3671a0)
endif()

# Build and install Anyness library                                         
add_library(LangulusAnyness ${LANGULUS_LIBRARY_TYPE}
    $<TARGET_OBJECTS:LangulusLogger>
    $<TARGET_OBJECTS:LangulusRTTI>
    $<$<BOOL:${LANGULUS_FEATURE_MANAGED_MEMORY}>:$<TARGET_OBJECTS:LangulusFractalloc>>
    $<$<BOOL:${LANGULUS_FEATURE_COMPRESSION}>:source/verbs/Compress.cpp>
    $<$<BOOL:${LANGULUS_FEATURE_ENCRYPTION}>:source/verbs/Encrypt.cpp>
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