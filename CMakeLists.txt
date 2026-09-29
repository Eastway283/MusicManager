cmake_minimum_required(VERSION 3.20)
project(music_manager C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(FLTK REQUIRED)
find_package(Threads REQUIRED)

add_executable(music_manager
    src/main.cpp
    src/db/Database.cpp
    src/db/Statement.cpp
    src/db/Migration.cpp
    src/services/AuthService.cpp
    src/services/CatalogService.cpp
    src/services/ConcertService.cpp
    src/services/ContractService.cpp
    src/ui/App.cpp
    src/ui/Windows.cpp
    src/ui/Panels.cpp
    src/ui/Modals.cpp
    src/util/Hash.cpp
    src/util/Date.cpp
    src/util/Status.cpp
    third_party/sqlite/sqlite3.c
)

target_include_directories(music_manager PRIVATE
    src
    third_party
    third_party/sqlite
    third_party/picosha2
    ${FLTK_INCLUDE_DIRS}
)

target_link_libraries(music_manager PRIVATE
    ${FLTK_LIBRARIES}
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

if(MSVC)
    target_compile_options(music_manager PRIVATE /utf-8)
else()
    target_compile_options(music_manager PRIVATE -Wall -Wextra -Wpedantic)
endif()
