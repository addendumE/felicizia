cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)


if (NOT DEFINED PROJECT_NAME)
	project(Felicizia)
endif (NOT DEFINED PROJECT_NAME)


# Inizializza la variabile
set(BUILD_VERSION "")

# Prova a ottenere il tag Git corrente
execute_process(
    COMMAND git describe --tags --exact-match
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_TAG_RESULT
    OUTPUT_VARIABLE GIT_TAG
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_TAG_RESULT EQUAL 0)
    set(BUILD_VERSION "${GIT_TAG}")
else()
    # Prova a ottenere l'hash breve del commit
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_HASH_RESULT
        OUTPUT_VARIABLE GIT_HASH
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(GIT_HASH_RESULT EQUAL 0)
        set(BUILD_VERSION "${GIT_HASH}")
    else()
        # Nessun repository Git valido → usa la data/ora della build
        string(TIMESTAMP BUILD_DATE "%Y%m%d-%H%M%S")
        set(BUILD_VERSION "${BUILD_DATE}")
    endif()
endif()

# Mostra la versione durante la build
message(STATUS "Build version: ${BUILD_VERSION}")

# Definisci la macro per il compilatore
add_compile_definitions(GIT_VERSION=\"${BUILD_VERSION}\")


get_property(target_names DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
message(target_names="${target_names}")

add_custom_command(
  TARGET Felicizia.elf  PRE_BUILD
  COMMAND ../makefsdata
  VERBATIM)