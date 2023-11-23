cmake_minimum_required(VERSION 3.24)
project(LangulusAnyness
    VERSION         1.0.0
    DESCRIPTION     "Langulus container library"
    HOMEPAGE_URL    https://langulus.com
)

# Check if this project is built as standalone, or a part of something else 
if(PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/lib)

    include(LangulusUtilities.cmake)

    # Add Langulus::Core/Logger/RTTI/Fractalloc/SIMD libraries              
    fetch_langulus_module(Core   GIT_TAG 0097bd180a5c625f88db0db0514bbf93aff53c7a)
    fetch_langulus_module(Logger GIT_TAG b63513abb9fb3014107335949e17e41e6e8a334b)
    fetch_langulus_module(RTTI   GIT_TAG f7c610255b819b99a6adcdefa970387490093c25)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG 17a8741cfa252b30e20257bb9d953d89278c4039)
    endif()
    fetch_langulus_module(SIMD   GIT_TAG 3cfb3bd47a8e0d610e0d473d74fbbde14dcea298)
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