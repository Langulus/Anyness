cmake_minimum_required(VERSION 3.24)
project(LangulusAnyness
    VERSION         1.0.0
    DESCRIPTION     "Langulus container library"
    HOMEPAGE_URL    https://langulus.com
)

# Check if this project is built as standalone, or a part of something else 
if(PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
    include(LangulusUtilities.cmake)

    # Add Langulus::Core/Logger/RTTI/Fractalloc/SIMD libraries              
    fetch_langulus_module(Core   GIT_TAG 21cf8d630324518797087a811860b102daf27d08)
    fetch_langulus_module(Logger GIT_TAG 3314fbe3bf5611a5bd9ee649f2a6ba5c43f6b4c4)
    fetch_langulus_module(RTTI   GIT_TAG 7221c92d1554521435684630af6f5e5800c3102a)
    if(LANGULUS_FEATURE_MANAGED_MEMORY)
        fetch_langulus_module(Fractalloc GIT_TAG 8ff8e85a09f35a406a6578e383b0a727a622f691)
    endif()
    fetch_langulus_module(SIMD   GIT_TAG 57e150d)
endif()

# Build and install Anyness library                                         
add_library(LangulusAnyness ${LANGULUS_LIBRARY_TYPE}
    $<TARGET_OBJECTS:LangulusLogger>
    $<TARGET_OBJECTS:LangulusRTTI>
    $<$<BOOL:${LANGULUS_FEATURE_MANAGED_MEMORY}>:$<TARGET_OBJECTS:LangulusFractalloc>>
    $<$<BOOL:${LANGULUS_FEATURE_COMPRESSION}>:source/verbs/Compress.cpp>
    $<$<BOOL:${LANGULUS_FEATURE_ENCRYPTION}>:source/verbs/Encrypt.cpp>
)

target_include_directories(LangulusAnyness
    PUBLIC	    include
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