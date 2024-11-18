cmake_minimum_required(VERSION 3.26)
project(PyFastUtil CXX C)

set(CMAKE_CXX_STANDARD 23)

#add_custom_target(Build DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/timestamp DEPENDS ${SOURCE_FILES} DEPENDS pyfastutil)
#add_custom_command(
#        DEPENDS Build
#        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/timestamp
#        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/timestamp
#        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/.venv/Scripts/activate
#        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py build_ext --inplace
#        COMMAND deactivate
#        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

include_directories(${Python3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pyfastutil/src)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pyfastutil/src/*")
add_library(MODULE ${SOURCE_FILES})

Python3_add_library(pyfastutil MODULE ${SOURCE_FILES})

target_link_libraries(pyfastutil PRIVATE ${Python3_LIBRARIES})

find_package(Python REQUIRED Interpreter Development)
