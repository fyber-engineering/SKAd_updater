find_package(CURL)
include_directories(${CURL_INCLUDE_DIRS})

include(Dependencies)

cmake_policy(SET CMP0042 NEW)

set(ALL_SRCS CACHE INTERNAL FORCE)

list(APPEND MAIN_SOURCES
        ${XML_LIB_SOURCES}
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        ${PROJECT_SOURCE_DIR}/src/cli.cpp
        ${PROJECT_SOURCE_DIR}/src/cli.h
        ${PROJECT_SOURCE_DIR}/src/Plist.cpp
        ${PROJECT_SOURCE_DIR}/src/Plist.h
        ${PROJECT_SOURCE_DIR}/src/exit_message.h
        ${PROJECT_SOURCE_DIR}/src/common.h
        ${PROJECT_SOURCE_DIR}/src/PodFile.cpp
        ${PROJECT_SOURCE_DIR}/src/PodFile.h
        ${PROJECT_SOURCE_DIR}/src/ManagerApi.cpp
        ${PROJECT_SOURCE_DIR}/src/ManagerApi.h
        )


add_executable(${MAIN_PROJECT_NAME} ${MAIN_SOURCES})

target_link_libraries(${MAIN_PROJECT_NAME} PRIVATE cpr::cpr ${CURL_LIBRARY})

set_source_files_properties(
        ${MAIN_SOURCES}
        PROPERTIES
        COMPILE_FLAGS "-Wall -Wno-long-long -pedantic"
)

target_compile_definitions(${MAIN_PROJECT_NAME} PUBLIC ${MAIN_PROJECT_NAME}_VERSION="${PROJECT_VERSION}")
