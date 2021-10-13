cmake_minimum_required(VERSION 3.20)

project(commandline VERSION 1.0)

# set content value
set(DEFAULT_PPID "" CACHE STRING "ppid")
set(DEFAULT_LOG_FILE "log.txt" CACHE STRING "log file")
add_definitions(
	-DPROJECT_VERSION="${PROJECT_VERSION}"
	-DDEFAULT_PPID="${DEFAULT_PPID}"
	-DDEFAULT_LOG_FILE="${DEFAULT_LOG_FILE}"
	)

# find pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
add_definitions(-DG_LOG_USE_STRUCTURED)
include_directories(${GLIB_INCLUDE_DIRS})

# find curl
find_package(CURL)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} ${GLIB_LIBRARIES})

# find doxygen
find_package(Doxygen)
if (DOXYGEN_FOUND)
	set(DOXYGEN_GENERATE_LATEX YES)
	set(DOXYGEN_EXTRACT_STATIC YES)
	message("Doxygen build start")
	doxygen_add_docs(docs ${PROJECT_SOURCE_DIR} ALL)
endif()
