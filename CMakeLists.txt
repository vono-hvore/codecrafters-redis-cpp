cmake_minimum_required(VERSION 3.13)

project(redis-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Lets clangd see the compiler flags
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)

add_executable(redis ${SOURCE_FILES})

target_link_libraries(redis PRIVATE asio asio::asio)
target_link_libraries(redis PRIVATE Threads::Threads)
target_include_directories(redis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
  enable_testing()
  find_package(GTest CONFIG REQUIRED)
  include(GoogleTest)

  add_executable(redis_tests tests/EventLoopTest.cpp src/EventLoop.cpp)
  target_include_directories(redis_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(redis_tests PRIVATE GTest::gtest_main Threads::Threads)
  gtest_discover_tests(redis_tests)
endif()
