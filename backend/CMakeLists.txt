cmake_minimum_required(VERSION 3.15)
project(RailwayTicketReservation VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dsa
    ${CMAKE_CURRENT_SOURCE_DIR}/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/api
)

set(DSA_SOURCES
    dsa/booking_ops.cpp
    dsa/seat_map.cpp
    dsa/train_ops.cpp
    dsa/validation.cpp
    dsa/waiting_queue.cpp
)

set(DB_SOURCES
    database/db_connection.cpp
    database/passenger_repo.cpp
    database/train_repo.cpp
    database/waiting_repo.cpp
)

# Target 1: Console Menu CLI
add_executable(railway_cli
    src/main.cpp
    src/menu.cpp
    ${DSA_SOURCES}
    ${DB_SOURCES}
)

# Target 2: REST API & Web Server
add_executable(railway_api
    api/server.cpp
    api/routes.cpp
    ${DSA_SOURCES}
    ${DB_SOURCES}
)

if(WIN32)
    target_link_libraries(railway_api PRIVATE ws2_32)
endif()
