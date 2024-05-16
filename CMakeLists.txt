cmake_minimum_required(VERSION 3.5)

project(SigSlot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions("-DWEBRTC_POSIX -DENABLE_STDCXX_SYNC=ON")


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thread-pool/include)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h**)

add_executable(SigSlot main.cpp ${SRC_FILES}
    core/event.cpp
    core/task_queue_base.cpp
    core/task_queue_manager.cpp
    core/task_queue_stdlib.cpp
    core/task_queue.cpp
    core/time_utils.cpp
    core/system_time.cpp
    core/warn_current_thread_is_deadlocked.cpp
    core/yield.cpp
    core/yield_policy.cpp)

# target_link_libraries(SigSlot -lpthread)

include(GNUInstallDirs)
install(TARGETS SigSlot
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
