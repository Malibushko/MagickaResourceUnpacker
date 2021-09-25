cmake_minimum_required(VERSION 3.5)

project(MagickaUnpacker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS third_party/MurmurHash2/MurmurHash2.h
            
            )
set(SOURCES main.cpp
            third_party/MurmurHash2/MurmurHash2.cpp
            )
            
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES zstr/1.0.4
                BUILD missing
                BASIC_SETUP
                CMAKE_TARGETS)

add_executable(MagickaUnpacker ${HEADERS} ${SOURCES} "src/SegmentedFileDecompressor.cpp")
target_include_directories(MagickaUnpacker PRIVATE src
                                                   third_party)

target_link_libraries(MagickaUnpacker PRIVATE CONAN_PKG::zstr)