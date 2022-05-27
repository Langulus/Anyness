cmake_minimum_required(VERSION 3.22)
include(cmake/DownloadProject.cmake)
project(Langulus_Anyness)
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
