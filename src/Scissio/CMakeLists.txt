file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_LIST_DIR}/main.cpp)

add_compile_options(/wd4251)

add_library(${PROJECT_NAME}Lib SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME}Lib 
	PUBLIC 
		glm 
		glad::glad 
		fmt::fmt 
		Cgltf
		Msgpack
	PRIVATE 
		glfw 
		Xml2 
		termcolor::termcolor 
		nanovg::nanovg 
		IL 
		Png 
		asio 
		asio::asio 
		unofficial::sqlite3::sqlite3 
		Nuklear
		Stb
)
target_compile_definitions(${PROJECT_NAME}Lib PRIVATE 
	SCISSIO_EXPORTS=1
	_WIN32_WINNT=0x0501
	_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1
	_CRT_SECURE_NO_WARNINGS=1
)
set_target_properties(${PROJECT_NAME}Lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(${PROJECT_NAME}Lib PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ArgAgg ${PROJECT_NAME}Lib)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)
