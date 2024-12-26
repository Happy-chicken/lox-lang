include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB PARSER_SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

message("parser library has been generated to ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
add_library(parser STATIC ${PARSER_SRC_LIST})

find_library(logger
             NAMES logger
             PATHS ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
target_link_libraries(parser PRIVATE ${logger})


