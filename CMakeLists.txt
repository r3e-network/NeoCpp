cmake_minimum_required(VERSION 3.20)
project(NeoCpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Include FetchContent for external dependencies
include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

# Fetch libcurl for HTTP requests
find_package(CURL REQUIRED)

# Fetch Catch2 for testing
if(BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()

FetchContent_MakeAvailable(json)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${json_SOURCE_DIR}/include
)

# Collect source files
file(GLOB_RECURSE NEOCPP_HEADERS include/neocpp/*.hpp)
file(GLOB_RECURSE NEOCPP_SOURCES src/*.cpp)

# Create library
add_library(neocpp ${NEOCPP_SOURCES} ${NEOCPP_HEADERS})

# Set include directories
target_include_directories(neocpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link libraries
target_link_libraries(neocpp PUBLIC
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
)

# Add json as interface dependency
target_include_directories(neocpp PUBLIC 
    $<BUILD_INTERFACE:${json_SOURCE_DIR}/include>
)

# Set properties
set_target_properties(neocpp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${NEOCPP_HEADERS}"
)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(neocpp PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -fPIC
        -Wno-deprecated-declarations
    )
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
        add_subdirectory(tests)
    endif()
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


# Export package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/NeoCppConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Export targets
install(TARGETS neocpp
    EXPORT NeoCppTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/neocpp
)

install(EXPORT NeoCppTargets
    FILE NeoCppTargets.cmake
    NAMESPACE NeoCpp::
    DESTINATION lib/cmake/NeoCpp
)

configure_file(cmake/NeoCppConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/NeoCppConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/NeoCppConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/NeoCppConfigVersion.cmake"
    DESTINATION lib/cmake/NeoCpp
)

# Print configuration summary
message(STATUS "NeoCpp Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Build shared libs: ${BUILD_SHARED_LIBS}")