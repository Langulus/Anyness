include(cmake/DownloadProject.cmake)

function(fetch_langulus_module NAME)
    message(STATUS "Fetching Langulus module: ${NAME}...")
    download_project(
	    PROJ                Langulus_${NAME}
	    GIT_REPOSITORY      https://github.com/Langulus/${NAME}.git
	    GIT_TAG             main
	    GIT_PROGRESS        TRUE
	    UPDATE_DISCONNECTED 1
    )
	add_subdirectory(${Langulus_${NAME}_SOURCE_DIR} ${Langulus_${NAME}_BINARY_DIR})
endfunction()