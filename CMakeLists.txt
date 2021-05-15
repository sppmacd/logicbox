project(LogicBox)
cmake_minimum_required(VERSION 3.0)

find_package(SFML 2.5.1 COMPONENTS graphics REQUIRED)

add_executable(logicbox
	"src/main.cpp"
)

target_link_libraries(logicbox sfml-graphics)
