cmake_minimum_required(VERSION 3.8)

project("QINP")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_BUILD_TYPE_RELEASE)

    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "-03")
endif()

add_executable(
    QINP
    "src/qinp"
)

set_property(TARGET QINP PROPERTY CXX_STANDARD 17)
