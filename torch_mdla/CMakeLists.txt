cmake_minimum_required(VERSION 3.7)

file(GLOB COMPILER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

# PYTORCH_DIR
IF(DEFINED ENV{PYTORCH_DIR})
  SET(PYTORCH_DIR $ENV{PYTORCH_DIR})
ENDIF()

IF ("${PYTORCH_DIR}" STREQUAL "")
  message(FATAL_ERROR "Please specify the PyTorch directory with -DPYTORCH_DIR=/path/to/pytorch/dir")
ENDIF()

message("Using PyTorch directory ${PYTORCH_DIR}")

link_directories(${PYTORCH_DIR}/lib)

add_subdirectory(pybind11)

pybind11_add_module(torchMDLA SHARED ${COMPILER_SRCS})
target_link_libraries(torchMDLA PUBLIC microndla torch torch_cpu)

target_include_directories(torchMDLA
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PYBIND11_INCLUDE_DIR}
  PUBLIC
    ${PYTORCH_DIR}/include
)

