cmake_minimum_required(VERSION 3.15)

project(ElmCANPlayer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#add_compile_definitions(DEBUG)
#add_compile_definitions(VIRTUAL_CAN_DEVICE)

add_subdirectory(stn1170-tools)

include_directories(${CMAKE_BINARY_DIR})
add_executable(elmplayer main.cpp PlayerGUI.cpp ./dialog.cpp CanController.cpp)
target_include_directories(elmplayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_include_directories(elmplayer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../stn1170-tools/serial/include)
target_link_libraries(elmplayer Qt5::Widgets Qt5::Core stnlib serial)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options(-Werror=vla)
	target_link_libraries(elmplayer pthread)
endif()