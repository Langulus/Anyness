include(FetchContent)

function(fetch_langulus_module NAME)
    if(NOT DEFINED LANGULUS_EXTERNAL_DIRECTORY)
        set(LANGULUS_EXTERNAL_DIRECTORY "${CMAKE_SOURCE_DIR}/external" CACHE PATH
            "Place where external dependencies will be downloaded and configured")
        message(WARNING "LANGULUS_EXTERNAL_DIRECTORY not defined, using default: ${LANGULUS_EXTERNAL_DIRECTORY}")
    endif()

    message(STATUS "Fetching external Langulus${NAME}...")
    FetchContent_Declare(
        Langulus${NAME}
        GIT_REPOSITORY  https://github.com/Langulus/${NAME}.git
        GIT_TAG         main
        GIT_SHALLOW     TRUE
        SOURCE_DIR      ${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src
        SUBBUILD_DIR    ${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-subbuild
        ${ARGN}
    )
    FetchContent_MakeAvailable(Langulus${NAME})
endfunction()

function(fetch_external_module NAME GIT_REPOSITORY REPO GIT_TAG TAG)
    if(NOT DEFINED LANGULUS_EXTERNAL_DIRECTORY)
        set(LANGULUS_EXTERNAL_DIRECTORY "${CMAKE_SOURCE_DIR}/external" CACHE PATH
            "Place where external dependencies will be downloaded and configured")
        message(WARNING "LANGULUS_EXTERNAL_DIRECTORY not defined, using default: ${LANGULUS_EXTERNAL_DIRECTORY}")
    endif()

    message(STATUS "Fetching external library ${NAME}...")
    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY  ${REPO}
        GIT_TAG         ${TAG}
        SOURCE_DIR      "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src"
        SUBBUILD_DIR    "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-subbuild"
        ${ARGN}
    )
    FetchContent_MakeAvailable(${NAME})
	
	string(TOLOWER ${NAME} LOWERCASE_NAME)
    set(${NAME}_SOURCE_DIR "${${LOWERCASE_NAME}_SOURCE_DIR}" CACHE INTERNAL "")
    set(${NAME}_BINARY_DIR "${${LOWERCASE_NAME}_BINARY_DIR}" CACHE INTERNAL "")
endfunction()

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
    if(WIN32 AND LANGULUS_SHARED_LIBRARIES)
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