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
    "src/QINP.cpp"
    "src/Token.cpp"
    "src/ExecCmd.cpp"
    "src/Symbols.cpp"
    "src/Program.cpp"
    "src/Datatype.cpp"
    "src/Tokenizer.cpp"
    "src/Statement.cpp"
    "src/ArgsParser.cpp"
    "src/NasmGenerator.cpp"
    "src/ExportComments.cpp"
    "src/ExportSymbolInfo.cpp"
    "src/ProgramGenerator.cpp"
    "src/OperatorPrecedence.cpp"
)

include_directories(
	QINP
	"src/"
)

set_property(TARGET QINP PROPERTY CXX_STANDARD 17)

if (WIN32)
    add_compile_definitions(QINP_PLATFORM_WINDOWS)
elseif (UNIX)
    add_compile_definitions(QINP_PLATFORM_UNIX)
elseif (APPLE)
    add_compile_definitions(QINP_PLATFORM_APPLE)
endif()