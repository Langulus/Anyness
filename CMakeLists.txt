cmake_minimum_required(VERSION 3.22)
project(Langulus.Anyness)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if this project is built as standalone, or a part of something else
if(PROJECT_IS_TOP_LEVEL)
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init -- cmake 
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(FetchLangulusModule)
    include(HandleLangulusFeatures)

    handle_langulus_features()

    fetch_langulus_module(Core)
    fetch_langulus_module(Logger)
    fetch_langulus_module(RTTI)

    add_compile_definitions(LANGULUS_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

# Build and install Anyness library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source/inner SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source/verbs SOURCE_FILES)
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