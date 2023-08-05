cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.81.0 REQUIRED COMPONENTS json log)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(game_server
	src/main.cpp
	src/http_server.cpp
	src/model.cpp
	src/util/error.cpp
	src/util/filesystem.cpp
	src/util/logging.cpp
	src/util/mime_type.cpp
	src/json_loader.cpp
)
target_link_libraries(game_server PRIVATE Threads::Threads ${Boost_LIBRARIES})
