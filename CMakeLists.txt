cmake_minimum_required(VERSION 3.24)
project(LangulusAnyness)

# Check if this project is built as standalone, or a part of something else 
if(PROJECT_IS_TOP_LEVEL OR NOT LANGULUS)
    # Populate the cmake directory											
    include(LangulusUtilities.cmake)
    langulus_fetch_cmake()

    # Handle feature flags and global definitions/requirements				
    include(HandleLangulusFeatures)
    handle_langulus_features()

    # Add Langulus::Core/Logger/RTTI/SIMD libraries                         
    include(FetchLangulusModule)
    fetch_langulus_module(Core)
    fetch_langulus_module(Logger)
    fetch_langulus_module(RTTI)
    fetch_langulus_module(SIMD)
endif()

# Build and install Anyness library                                         
if(LANGULUS_FEATURE_MANAGED_REFLECTION)
    set(LANGULUS_ANYNESS_SOURCES source/fractalloc/Allocator.cpp)
endif()

add_library(LangulusAnyness ${LANGULUS_LIBRARY_TYPE}
    $<TARGET_OBJECTS:LangulusLogger>
    $<TARGET_OBJECTS:LangulusRTTI>
    source/verbs/Compress.cpp
    source/verbs/Encrypt.cpp
    ${LANGULUS_ANYNESS_SOURCES}
)

target_include_directories(LangulusAnyness
    PUBLIC	    include
                $<TARGET_PROPERTY:LangulusLogger,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:LangulusRTTI,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(LangulusAnyness
    PUBLIC      LangulusCore
                fmt
)

target_compile_definitions(LangulusAnyness
    PRIVATE     LANGULUS_EXPORT_ALL
)

if(LANGULUS_ENABLE_TESTING)
	add_subdirectory(test)
    langulus_copy_dlls(LangulusAnynessTest ON LangulusAnyness)
endif()