cmake_minimum_required(VERSION 3.23)

project(ingress-drone-explorer)

option(USE_STATIC_LIBS "Prefer to link static libraries" OFF)

if(USE_STATIC_LIBS)
    set(Boost_USE_STATIC_LIBS ON)
    if(MSVC)
        set(Boost_USE_STATIC_RUNTIME ON)
    endif()
endif()

find_package(Boost REQUIRED
    COMPONENTS
    json
    program_options
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(
    ${Boost_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB_RECURSE CXX_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE CXX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${CXX_HEADERS} ${CXX_SOURCE})

target_link_libraries(${PROJECT_NAME}
    Boost::json
    Boost::program_options
)

if(USE_STATIC_LIBS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(${PROJECT_NAME}
            stdc++ -static
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_link_libraries(${PROJECT_NAME}
            c++ -static
        )
    elseif(MSVC)
        set_target_properties(${PROJECT_NAME}
            PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded"
        )
    endif()
endif()

install(TARGETS ${PROJECT_NAME})