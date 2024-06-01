include(FetchContent)

function(fetch_langulus_module NAME GIT_TAG TAG)
    if(NOT DEFINED LANGULUS_EXTERNAL_DIRECTORY)
        set(LANGULUS_EXTERNAL_DIRECTORY "${CMAKE_SOURCE_DIR}/external" CACHE PATH
            "Place where external dependencies will be downloaded")
        message(WARNING "LANGULUS_EXTERNAL_DIRECTORY not defined, using default: ${LANGULUS_EXTERNAL_DIRECTORY}")
    endif()

    message(STATUS "Fetching external Langulus::${NAME}...")
    FetchContent_Declare(
        Langulus${NAME}
        GIT_REPOSITORY  https://github.com/Langulus/${NAME}.git
        GIT_TAG         ${TAG}
        SOURCE_DIR      "${LANGULUS_EXTERNAL_DIRECTORY}/${NAME}-src"
        SUBBUILD_DIR    "${CMAKE_BINARY_DIR}/external/${NAME}-subbuild"
        ${ARGN}
		EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(Langulus${NAME})
endfunction()

function(fetch_external_module NAME GIT_REPOSITORY REPO GIT_TAG TAG)
    if(NOT DEFINED LANGULUS_EXTERNAL_DIRECTORY)
        set(LANGULUS_EXTERNAL_DIRECTORY "${CMAKE_SOURCE_DIR}/external" CACHE PATH
            "Place where external dependencies will be downloaded")
        message(WARNING "LANGULUS_EXTERNAL_DIRECTORY not defined, using default: ${LANGULUS_EXTERNAL_DIRECTORY}")
    endif()

    message(STATUS "Fetching external library ${NAME}...")
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