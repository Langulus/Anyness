cmake_minimum_required(VERSION 3.22)
project(Langulus.Anyness)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if this project is built as standalone, or a part of something else
if(PROJECT_IS_TOP_LEVEL)
    if(LANGULUS_BUILD_SHARED_LIBRARIES)
        add_compile_definitions(LANGULUS_EXPORT_ALL)
        set(LANGULUS_LIBRARY_TYPE SHARED)
    else()
        set(LANGULUS_LIBRARY_TYPE STATIC)
    endif()

    include(LangulusUtilities.cmake)
    langulus_init_git_submodule(cmake)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(FetchLangulusModule)
    include(HandleLangulusFeatures)

    handle_langulus_features()

    fetch_langulus_module(Core)
    fetch_langulus_module(Logger)
    fetch_langulus_module(RTTI)
    fetch_langulus_module(SIMD)

    add_compile_definitions(LANGULUS_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

# Build and install Anyness library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source LANGULUS_ANYNESS_SOURCES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source/inner LANGULUS_ANYNESS_SOURCES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source/verbs LANGULUS_ANYNESS_SOURCES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source/blocks LANGULUS_ANYNESS_SOURCES)
add_library(Langulus.Anyness ${LANGULUS_LIBRARY_TYPE} ${LANGULUS_ANYNESS_SOURCES})

target_include_directories(Langulus.Anyness
    PUBLIC	    include
)

target_link_libraries(Langulus.Anyness
    PUBLIC      Langulus.Core
                Langulus.Logger
                Langulus.RTTI
                Langulus.SIMD
)