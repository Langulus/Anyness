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

function(langulus_import_framework)
    remove_definitions(-DLANGULUS_EXPORT_ALL)
    if(NOT WIN32)
        add_compile_options(-fvisibility=hidden)
    endif()
endfunction()

function(langulus_copy_dlls TO TARGET FROM)
    if(WIN32)
        foreach(element ${ARGN})
            add_custom_command(
                TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${element}>" "$<TARGET_FILE_DIR:${TARGET}>"
                COMMENT "Copying `$<TARGET_FILE:${element}>` to `$<TARGET_FILE_DIR:${TARGET}>`"
            )
        endforeach()
    endif()
endfunction()
