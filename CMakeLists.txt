cmake_minimum_required(VERSION 3.22)
include(cmake/FetchLangulusModule.cmake)
project(Langulus.Anyness)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure the features you require, all these are disabled by default
option(LANGULUS_ENABLE_SAFE_MODE 
    "Overrides additional error checking and sanity checks, \
    incurs a serious runtime overhead")

option(LANGULUS_ENABLE_PARANOIA 
    "Make sure that any unused memory is nullified, \
    incurs a serious runtime overhead")

option(LANGULUS_ENABLE_DEBUGGING 
    "Explicitly define debug mode, in the case that default flags are \
    not detected by some odd reason")

option(LANGULUS_FEATURE_NEWDELETE 
    "Overrides new/delete operators for anything statically linked to this library, \
    or provides LANGULUS_MONOPOLIZE_MEMORY() macro for you to use to override them, if dynamically linked. \
    This feature can be enabled only if LANGULUS_FEATURE_MANAGED_MEMORY is enabled")

option(LANGULUS_FEATURE_UTFCPP
    "Links to utfcpp library and provides UTF8, UTF16, UTF32 conversions")

option(LANGULUS_FEATURE_ZLIB
    "Links to zlib library and provides memory compression for use with any container of your liking")

option(LANGULUS_FEATURE_ENCRYPTION
    "Allows for memory encryption for use with any container of your liking")

option(LANGULUS_FEATURE_MANAGED_MEMORY
    "Memory will be pooled and recycled when freed, and you can safely push any kind of pointer, \
    as long as it was allocated by the memory manager, or by the overridden new/delete feature")

option(LANGULUS_FEATURE_MANAGED_REFLECTION
    "Reflections will be kept in a centralized location, when reflected, which speeds up \
    type comparisons, and allows you to dynamically modify the reflection at runtime. \
    This feature can be enabled only if LANGULUS_FEATURE_MANAGED_MEMORY is enabled")

option(LANGULUS_FEATURE_MEMORY_STATISTICS
    "Memory manager shall keep track of statistics, for the price of little overhead")

if(LANGULUS_ENABLE_SAFE_MODE)
    add_compile_definitions(LANGULUS_ENABLE_SAFE_MODE)
endif()
if(LANGULUS_ENABLE_PARANOIA)
    add_compile_definitions(LANGULUS_ENABLE_PARANOIA)
endif()
if(LANGULUS_ENABLE_DEBUGGING)
    add_compile_definitions(LANGULUS_ENABLE_DEBUGGING)
endif()
if(LANGULUS_FEATURE_NEWDELETE)
    add_compile_definitions(LANGULUS_ENABLE_FEATURE_NEWDELETE)
endif()
if(LANGULUS_FEATURE_UTFCPP)
    add_compile_definitions(LANGULUS_ENABLE_FEATURE_UTFCPP)
    #TODO download_project utfcpp
endif()
if(LANGULUS_FEATURE_ZLIB)
    add_compile_definitions(LANGULUS_ENABLE_FEATURE_ZLIB)
    #TODO download_project zlib
endif()
if(LANGULUS_FEATURE_ENCRYPTION)
    add_compile_definitions(LANGULUS_ENABLE_FEATURE_ENCRYPTION)
endif()
if(LANGULUS_FEATURE_MANAGED_MEMORY)
    add_compile_definitions(LANGULUS_ENABLE_FEATURE_MANAGED_MEMORY)
endif()
if(LANGULUS_FEATURE_MANAGED_REFLECTION)
    add_compile_definitions(LANGULUS_ENABLE_MANAGED_REFLECTION)
endif()
if(LANGULUS_FEATURE_MEMORY_STATISTICS)
    add_compile_definitions(LANGULUS_ENABLE_FEATURE_MEMORY_STATISTICS)
endif()

# Check if this project is built as standalone, or a part of something else
if(PROJECT_IS_TOP_LEVEL)
    # We're building Anyness as standalone
    fetch_langulus_module(Core)
    fetch_langulus_module(Logger)
    fetch_langulus_module(RTTI)

    enable_testing()
    add_subdirectory(test)
endif()

# Build and install Anyness library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/inner SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/verbs SOURCE_FILES)
add_library(Langulus.Anyness ${SOURCE_FILES})

if (MSVC)
    target_compile_options(Langulus.Anyness PRIVATE /utf-8)
endif()

target_include_directories(Langulus.Anyness
    PUBLIC	    include
)

target_link_libraries(Langulus.Anyness
    PUBLIC      Langulus.RTTI
)