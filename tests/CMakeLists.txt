set(TEST_PROJECT_NAME tests)

project(${TEST_PROJECT_NAME})

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
)

message("Fetching googletest")
FetchContent_MakeAvailable(googletest)


include_directories(${gtest_SOURCE_DIR}/include ${gmock_SOURCE_DIR}/include)


add_executable(${TEST_PROJECT_NAME}_run end2end.cpp)

target_include_directories(${TEST_PROJECT_NAME}_run PUBLIC ${gtest_SOURCE_DIR}/include ${gmock_SOURCE_DIR}/include)
target_link_libraries(${TEST_PROJECT_NAME}_run gtest gtest_main gmock gmock_main)

target_compile_definitions(${TEST_PROJECT_NAME}_run PRIVATE
        ${MAIN_PROJECT_NAME}_VERSION="${${MAIN_PROJECT_NAME}_VERSION}"
        ${MAIN_PROJECT_NAME}_BIN="${MAIN_PROJECT_NAME_BIN}"
        )

add_test(
        ${TEST_PROJECT_NAME}_run
        ${TEST_PROJECT_NAME}_run
)


