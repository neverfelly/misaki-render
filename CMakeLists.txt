cmake_minimum_required(VERSION 3.17)
project(aspirin)

set(CMAKE_CXX_STANDARD 17)

find_package(Eigen3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)
find_package(embree 3 REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

option(APR_ENABLE_EMBREE  "Use Embree for ray tracing intersection" ON)

if (APR_ENABLE_EMBREE)
    add_compile_options(-DAPR_ENABLE_EMBREE)
endif ()

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(include)

add_subdirectory(src)