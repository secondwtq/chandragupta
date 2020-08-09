cmake_minimum_required(VERSION 3.1)
project(Chandragupta)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find_package(asio REQUIRED)
add_compile_definitions(SPDLOG_FMT_EXTERNAL)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_executable(chandragupta main.cxx Socks5Impl.cxx Client.cxx)
target_link_libraries(chandragupta PRIVATE Threads::Threads)
target_link_libraries(chandragupta PRIVATE spdlog::spdlog)
target_link_libraries(chandragupta PRIVATE fmt::fmt)
