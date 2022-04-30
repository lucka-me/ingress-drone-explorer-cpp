cmake_minimum_required(VERSION 3.23)

project(ingress-drone-explorer)

set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED
    COMPONENTS
    json
    program_options
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(
    ${Boost_INCLUDE_DIR}

    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB_RECURSE CXX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${CXX_SOURCE})

target_link_libraries(${PROJECT_NAME}
    Boost::json
    Boost::program_options
)