function(langulus_init_git_submodule NAME)
	if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}/.git" )
        # Submodule hasn't been initialized yet, so call git submodule update --init on it
        message(STATUS "Initializing submodule: ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}...")
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init -- ${NAME} 
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND_ERROR_IS_FATAL ANY
        )
	endif()
endfunction()

function(langulus_copy_dlls TARGET ON THIS)
    if(WIN32 AND LANGULUS_BUILD_SHARED_LIBRARIES)
        add_custom_command(
            TARGET ${THIS} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_RUNTIME_DLLS:${TARGET}>" "$<TARGET_FILE_DIR:${TARGET}>"
            COMMAND_EXPAND_LISTS
        )
    endif()
endfunction()

function(langulus_copy_dlls_advanced THIS TO TARGET FROM)
    if(WIN32)
        add_dependencies(${TARGET} ${ARGN})
        foreach(element ${ARGN})
            add_custom_command(
                TARGET ${THIS} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${element}>" "$<TARGET_FILE_DIR:${TARGET}>"
                COMMENT "Copying `$<TARGET_FILE:${element}>` to `$<TARGET_FILE_DIR:${TARGET}>`"
            )
        endforeach()
    endif()
endfunction()

include(FetchContent)
function(langulus_fetch_cmake)
    message(STATUS "Fetching cmake scripts...")
    FetchContent_Declare(
        LangulusCMake
        GIT_REPOSITORY  https://github.com/Langulus/CMake.git
        GIT_TAG         main
        GIT_SHALLOW     TRUE
        SOURCE_DIR      "${CMAKE_SOURCE_DIR}/external/cmake-src"
        SUBBUILD_DIR    "${CMAKE_SOURCE_DIR}/external/cmake-subbuild"
        ${ARGN}
    )
    FetchContent_MakeAvailable(LangulusCMake)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/cmake-src")
    set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endfunction()