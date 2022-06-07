cmake_minimum_required(VERSION 3.22)
include(cmake/DownloadProject.cmake)
project(Langulus_Anyness)

# Configure the features you require, all these are disabled by default
option(LANGULUS_ENABLE_SAFE_MODE 
    "Overrides additional error checking and sanity checks, \
    incurs a serious runtime overhead" ON)

option(LANGULUS_ENABLE_PARANOIA 
    "Make sure that any unused memory is nullified, \
    incurs a serious runtime overhead")

option(LANGULUS_ENABLE_DEBUGGING 
    "Explicitly define debug mode, in the case that default flags are \
    not detected by some odd reason")

option(LANGULUS_FEATURE_NEWDELETE 
    "Overrides new/delete operators for anything statically linked to this library, \
    or provides LANGULUS_MONOPOLIZE_MEMORY() macro for you to use to override them, if dynamically linked")

option(LANGULUS_FEATURE_UTFCPP
    "Links to utfcpp library and provides UTF8, UTF16, UTF32 conversions")

option(LANGULUS_FEATURE_ZLIB
    "Links to zlib library and provides memory compression for use with any container of your liking")

option(LANGULUS_FEATURE_ENCRYPTION
    "Allows for memory encryption for use with any container of your liking")

option(LANGULUS_FEATURE_MANAGED_MEMORY
    "Memory will be pooled and recycled when freed, and you can safely push any kind of pointer, \
    as long as it was allocated by the memory manager, or by the overridden new/delete feature" ON)

option(LANGULUS_FEATURE_MANAGED_REFLECTION
    "Reflections will be kept in a centralized location, when reflected, which speeds up \
    type comparisons, and allows you to dynamically modify the reflection at runtime")

if(LANGULUS_ENABLE_SAFE_MODE)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_SAFE_MODE)
endif()
if(LANGULUS_ENABLE_PARANOIA)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_PARANOIA)
endif()
if(LANGULUS_ENABLE_DEBUGGING)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_DEBUGGING)
endif()
if(LANGULUS_FEATURE_NEWDELETE)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_FEATURE_NEWDELETE)
endif()
if(LANGULUS_FEATURE_UTFCPP)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_FEATURE_UTFCPP)
    #TODO download_project utfcpp
endif()
if(LANGULUS_FEATURE_ZLIB)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_FEATURE_ZLIB)
    #TODO download_project zlib
endif()
if(LANGULUS_FEATURE_ENCRYPTION)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_FEATURE_ENCRYPTION)
endif()
if(LANGULUS_FEATURE_MANAGED_MEMORY)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_MANAGED_MEMORY)
endif()
if(LANGULUS_FEATURE_MANAGED_REFLECTION)
    ADD_DEFINITIONS(-DLANGULUS_ENABLE_MANAGED_REFLECTION)
endif()

# Add source
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/inner SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/verbs SOURCE_FILES)

# Check if this project is built as standalone, or a part of something else
if(PROJECT_IS_TOP_LEVEL)
    # We're building Anyness as standalone
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    enable_testing()

    # Configure Langulus.Core
    message(STATUS "Configuring external project: Langulus.Core...")
    download_project(
	    PROJ                Langulus_Core
	    GIT_REPOSITORY      https://github.com/Langulus/Core.git
	    GIT_TAG             main
	    GIT_PROGRESS        TRUE
	    UPDATE_DISCONNECTED 1
    )
    include_directories(${Langulus_Core_SOURCE_DIR}/include)
endif()

# Build and install Anyness library
add_library(Langulus.Anyness ${SOURCE_FILES})
target_include_directories(Langulus.Anyness
    PRIVATE     ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE	${CMAKE_CURRENT_SOURCE_DIR}/include/
)

# Testing
add_subdirectory(test)