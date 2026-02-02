cmake_minimum_required(VERSION 3.16)
project(test_spk_host LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5 REQUIRED COMPONENTS Quick Gui Qml Network)

add_executable(test_spk_host
    main.cpp
    TriggerServer.cpp
)

target_link_libraries(test_spk_host
    Qt5::Quick
    Qt5::Gui
    Qt5::Qml
    Qt5::Network
)
