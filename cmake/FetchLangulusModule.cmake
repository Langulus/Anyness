include(cmake/DownloadProject.cmake)

function(fetch_langulus_module NAME)
    message(STATUS "Fetching Langulus module: ${NAME}...")
    download_project(
	    PROJ                Langulus_${NAME}
	    GIT_REPOSITORY      https://github.com/Langulus/${NAME}.git
	    GIT_TAG             main
	    UPDATE_DISCONNECTED 1
    )

	add_subdirectory(${Langulus_${NAME}_SOURCE_DIR} ${Langulus_${NAME}_BINARY_DIR})
	set(Langulus_${NAME}_SOURCE_DIR ${Langulus_${NAME}_SOURCE_DIR} PARENT_SCOPE)
	set(Langulus_${NAME}_BINARY_DIR ${Langulus_${NAME}_BINARY_DIR} PARENT_SCOPE)
endfunction()