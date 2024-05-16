cmake_minimum_required(VERSION 3.5)

project(SigSlot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thread-pool/include)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h**)

add_executable(SigSlot main.cpp ${SRC_FILES}
    event.cpp task_queue_base.cpp task_queue_manager.cpp task_queue_std.cpp task_queue.cpp)

include(GNUInstallDirs)
install(TARGETS SigSlot
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
