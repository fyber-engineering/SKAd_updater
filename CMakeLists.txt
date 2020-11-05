cmake_minimum_required(VERSION 3.18)

set(MAIN_PROJECT_NAME skad_updater)

project(${MAIN_PROJECT_NAME} VERSION 1.0.4
        DESCRIPTION "Update the SKAdNetworks in your plist file"
        LANGUAGES CXX)

# Global settings
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(MAIN_PROJECT_NAME_BIN ${PROJECT_BINARY_DIR}/bin)

message("Cmake Build Type" = ${CMAKE_BUILD_TYPE})

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif ()

add_subdirectory(cmake/Format.cmake)
add_subdirectory(src)

set_target_properties(${MAIN_PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${MAIN_PROJECT_NAME_BIN}"
        )


#############
# Info
#############
message("Binary location: ${MAIN_PROJECT_NAME_BIN}")


###############
# build tests
##############

option(PACKAGE_TESTS "Build the tests" ON)
if (PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

############
# package
############

add_custom_target(package ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/docs ${PROJECT_BINARY_DIR}/docs
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/LICENSE ${PROJECT_BINARY_DIR}/LICENSE
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/completions
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/homebrew/completions.bash ${PROJECT_BINARY_DIR}/completions
        COMMAND ${CMAKE_COMMAND} -E tar cfvz ${MAIN_PROJECT_NAME}-${${MAIN_PROJECT_NAME}_VERSION}.tar.gz ${PROJECT_BINARY_DIR}/LICENSE ${MAIN_PROJECT_NAME_BIN} ${PROJECT_BINARY_DIR}/docs ${PROJECT_BINARY_DIR}/completions
        COMMAND ${PROJECT_SOURCE_DIR}/generate_brew_formula.sh ${PROJECT_VERSION} ${MAIN_PROJECT_NAME} ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
        DEPENDS ${MAIN_PROJECT_NAME}
        )
