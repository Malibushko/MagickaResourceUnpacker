cmake_minimum_required(VERSION 3.5)

project(MagickaUnpacker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS third_party/MurmurHash2/MurmurHash2.h
            src/Unpacker.h
            )
set(SOURCES main.cpp
            third_party/MurmurHash2/MurmurHash2.cpp
            src/Unpacker.cpp
            )

set(REPROC++ ON)
add_subdirectory(third_party/reproc)

add_executable(MagickaUnpacker ${HEADERS} ${SOURCES})
target_include_directories(MagickaUnpacker PRIVATE src
                                                   third_party
                                                   third_party/reproc/reproc++/include)
target_link_libraries(MagickaUnpacker PRIVATE reproc++)
