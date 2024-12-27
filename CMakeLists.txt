cmake_minimum_required(VERSION 3.16)
project(clox-tree-walk CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Debug")

add_subdirectory(src/logger)
add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/interpreter)

#file(GLOB SRC_lexer ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer/*.cpp)
#file(GLOB SRC_logger ${CMAKE_CURRENT_SOURCE_DIR}/src/logger/*.cpp)
#file(GLOB SRC_interpreter ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter/*.cpp)
file(GLOB SRC_irgenerator ${CMAKE_CURRENT_SOURCE_DIR}/src/irgenerator/*.cpp)

file(GLOB SRC_utils ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp)
file(GLOB SRC_builtins ${CMAKE_CURRENT_SOURCE_DIR}/src/builtins/*.cpp)
file(GLOB SRC_internals ${CMAKE_CURRENT_SOURCE_DIR}/src/internals/*.cpp)

set(SRC ${SRC_irgenerator} ${SRC_utils} ${SRC_builtins} ${SRC_internals})
find_package(LLVM REQUIRED CONFIG)

include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

#link_libraries(logger lexer parser interpreter)

add_executable(main main.cpp lox.cpp vm.cpp ${SRC})

llvm_map_components_to_libnames(LLVM_LIBS support core irreader)
target_link_libraries(main logger lexer parser interpreter ${LLVM_LIBS})
target_compile_options(main PRIVATE -fstandalone-debug)




 