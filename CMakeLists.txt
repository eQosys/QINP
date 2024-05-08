cmake_minimum_required(VERSION 3.8)

project("qinp")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_BUILD_TYPE_RELEASE)

    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

add_executable(
    qinp
    "src/QINP.cpp"
    "src/Program.cpp"
    "src/utility/Platform.cpp"
    "src/utility/FileReader.cpp"
    "src/utility/HelpPrinter.cpp"
    "src/utility/Architecture.cpp"
    "src/utility/CmdArgParser.cpp"
    "src/utility/StringHelpers.cpp"
    "src/utility/EnvironmentParser.cpp"
    "src/errors/ErrorPrinter.cpp"
    )

include_directories(
	qinp
	"src/"
)

add_subdirectory("vendor/Qrawlr/libQrawlr")

target_link_libraries(
    qinp
    libqrawlr
)

set_property(TARGET qinp PROPERTY CXX_STANDARD 17)

if (WIN32)
    add_compile_definitions(QINP_PLATFORM_WINDOWS)
elseif (UNIX)
    add_compile_definitions(QINP_PLATFORM_UNIX)
elseif (APPLE)
    add_compile_definitions(QINP_PLATFORM_APPLE)
endif()

add_compile_definitions(QINP_ARCHITECTURE_X86)