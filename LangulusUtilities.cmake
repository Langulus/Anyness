include(FetchContent)


# Utility for fetching Langulus libraries using FetchContent					
function(fetch_langulus_module NAME GIT_TAG TAG)
	if (LANGULUS)
		message(FATAL_ERROR "You can't fetch Langulus::${NAME}, because this build \
		indicates LANGULUS is being build along your project. The library you're \
		trying to fetch should already be available locally.")
	endif()

    if(NOT DEFINED LANGULUS_EXTERNAL_DIRECTORY)
        set(LANGULUS_EXTERNAL_DIRECTORY "${CMAKE_SOURCE_DIR}/external" CACHE PATH
            "Place where external dependencies will be downloaded")
        message(WARNING "LANGULUS_EXTERNAL_DIRECTORY not defined, using default: \
		${LANGULUS_EXTERNAL_DIRECTORY}")
    endif()

	# Completely avoid downloading or updating anything, once the appropriate	
	# folder exists																
	string(TOUPPER ${NAME} UPPERCASE_NAME)
	if (EXISTS "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src")
		set(FETCHCONTENT_SOURCE_DIR_LANGULUS${UPPERCASE_NAME} "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src" CACHE INTERNAL "" FORCE)
		message(STATUS "Reusing external library Langulus::${NAME}... \
		(delete ${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src manually in order to redownload)")
	else()
		unset(FETCHCONTENT_SOURCE_DIR_LANGULUS${UPPERCASE_NAME} CACHE)
		message(STATUS "Downloading external library Langulus::${NAME}...")
	endif()

    FetchContent_Declare(
        Langulus${NAME}
        GIT_REPOSITORY  https://github.com/Langulus/${NAME}.git
        GIT_TAG         ${TAG}
        SOURCE_DIR      "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src"
        SUBBUILD_DIR    "${CMAKE_BINARY_DIR}/external/${NAME}-subbuild"
        ${ARGN}
    )
    FetchContent_MakeAvailable(Langulus${NAME})
endfunction()


# Utility for fetching external libraries using FetchContent					
function(fetch_external_module NAME GIT_REPOSITORY REPO GIT_TAG TAG)
    if(NOT DEFINED LANGULUS_EXTERNAL_DIRECTORY)
        set(LANGULUS_EXTERNAL_DIRECTORY "${CMAKE_SOURCE_DIR}/external" CACHE PATH
            "Place where external dependencies will be downloaded")
        message(WARNING "LANGULUS_EXTERNAL_DIRECTORY not defined, \
		using default: ${LANGULUS_EXTERNAL_DIRECTORY}")
    endif()

	# Completely avoid downloading or updating anything, once the appropriate	
	# folder exists																
	string(TOUPPER ${NAME} UPPERCASE_NAME)
	if (EXISTS "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src")
		set(FETCHCONTENT_SOURCE_DIR_${UPPERCASE_NAME} "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src" CACHE INTERNAL "" FORCE)
		message(STATUS "Reusing external library ${NAME}... \
		(delete ${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src manually in order to redownload)")
	else()
		unset(FETCHCONTENT_SOURCE_DIR_${UPPERCASE_NAME} CACHE)
		message(STATUS "Downloading external library ${NAME}...")
	endif()

    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY  ${REPO}
        GIT_TAG         ${TAG}
        SOURCE_DIR      "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src"
        SUBBUILD_DIR    "${CMAKE_BINARY_DIR}/external/${NAME}-subbuild"
        ${ARGN}
		EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(${NAME})
	
	string(TOLOWER ${NAME} LOWERCASE_NAME)
    set(${NAME}_SOURCE_DIR "${${LOWERCASE_NAME}_SOURCE_DIR}" CACHE INTERNAL "${NAME} source directory")
    set(${NAME}_BINARY_DIR "${${LOWERCASE_NAME}_BINARY_DIR}" CACHE INTERNAL "${NAME} binary directory")
endfunction()
