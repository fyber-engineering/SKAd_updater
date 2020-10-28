
include(FetchContent)

########################
# CLI
########################
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts
        GIT_TAG 302302b30839505703d37fb82f536c53cf9172fa # v2.2.1
)

message("Fetching cxxopts")
FetchContent_GetProperties(cxxopts)
if (NOT cxxopts_POPULATED)
    FetchContent_Populate(cxxopts)

    include_directories(${cxxopts_SOURCE_DIR}/include)
endif ()

########################
# Logging
########################
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG cbe9448650176797739dbab13961ef4c07f4290f # v1.8.1
)

message("Fetching spdlog")
FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)

    include_directories(${spdlog_SOURCE_DIR}/include)
endif ()


########################
# Xml
########################
FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml
        GIT_TAG 3c59df555b93a7650ae0056da34bacde93f4ef8f # v1.10
)

message("Fetching pugixml")
FetchContent_GetProperties(pugixml)
if (NOT pugixml_POPULATED)
    FetchContent_Populate(pugixml)

    include_directories(${pugixml_SOURCE_DIR}/src)
    set(XML_LIB_SOURCES ${pugixml_SOURCE_DIR}/src/pugixml.cpp)
endif ()

########################
# HTTP client
########################
set(BUILD_CPR_TESTS OFF)
set(USE_SYSTEM_CURL ON)
set(CMAKE_USE_OPENSSL OFF)

FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/whoshuu/cpr.git
        GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) #  1.5.0

message("Fetching cpr")
FetchContent_MakeAvailable(cpr)


########################
# Json
########################
FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson
        GIT_TAG f54b0e47a08782a6131cc3d60f94d038fa6e0a51 # v1.1.0
)

message("Fetching rapidjson")
FetchContent_GetProperties(rapidjson)
if (NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)

    include_directories(${rapidjson_SOURCE_DIR}/include)
endif ()
