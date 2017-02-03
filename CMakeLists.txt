cmake_minimum_required(VERSION 2.8)
project(waltham_study)

find_package(PkgConfig)
pkg_check_modules(WALTHAM REQUIRED waltham)

include_directories(${WALTHAM_INCLUDE_DIRS})
link_directories(${WALTHAM_LIBRARY_DIRS})

add_executable(server server.cpp)
target_link_libraries(server ${WALTHAM_LIBRARIES} -lboost_system)

add_executable(client client.cpp)
target_link_libraries(client ${WALTHAM_LIBRARIES} -lboost_system)
