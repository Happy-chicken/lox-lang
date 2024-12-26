include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB LOGGER_SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)


# 如果有源文件，创建静态库
if (LOGGER_SRC_LIST)
    message("logger library has been generated to ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    add_library(logger STATIC ${LOGGER_SRC_LIST})
else()
    message(FATAL_ERROR "No source files found for logger.")
endif()