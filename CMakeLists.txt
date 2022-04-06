cmake_minimum_required(VERSION 3.22)
project(Anyness)
add_subdirectory(test)

#Check if this project is used as a subdirectory
#Potential issue with this: https://gitlab.kitware.com/cmake/cmake/-/issues/18862
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	#This is the root of the CMakeLists, so enable testing here
	enable_testing()
endif()

#Build and install of the Anyness shared library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_FILES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/verbs SOURCE_FILES)
add_library(Anyness SHARED ${SOURCE_FILES})
target_include_directories(Anyness
	PRIVATE		${CMAKE_CURRENT_SOURCE_DIR}
	INTERFACE	${CMAKE_CURRENT_SOURCE_DIR}/include
)
install(TARGETS Anyness DESTINATION lib)

#Testing
add_test(NAME TestAnyness COMMAND TestAnyness)