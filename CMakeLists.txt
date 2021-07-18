cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH 
  "${CMAKE_CURRENT_LIST_DIR}/modules"
)

project(Scissio)

if(NOT MSVC)
  find_package(ZLIB REQUIRED)
  find_package(liblzma CONFIG REQUIRED)
else()
  add_compile_options(/EHsc)
endif()

find_package(ArgAgg REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(Xml2 REQUIRED)
find_package(termcolor CONFIG REQUIRED)
find_package(nanovg CONFIG REQUIRED)
find_package(Png REQUIRED)
find_package(Cgltf REQUIRED)
find_package(Msgpack REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(Nuklear REQUIRED)
find_package(Stb REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
