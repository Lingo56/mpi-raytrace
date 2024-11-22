cmake_minimum_required(VERSION 3.22.1)

project(mpi-raytrace CXX)

find_package(MPI REQUIRED CXX)

include(cmake/CPM.cmake)

# CPMAddPackage(
#   NAME fpng
#   GITHUB_REPOSITORY richgel999/fpng
#   GIT_TAG 925796543b9d26b8edfcdcecd94c1dac280f29fc
#   OPTIONS
#     "SSE TRUE"
# )
# if(NOT fpng_ADDED)
#   message(FATAL_ERROR "fpng not added")
# endif()

add_subdirectory(vendor/fpng)

add_library(dependencies INTERFACE)
target_link_libraries(dependencies INTERFACE MPI::MPI_CXX fpng)
# target_include_directories(dependencies INTERFACE "${fpng_SOURCE_DIR}")

add_executable(mpi-raytrace)
target_include_directories(mpi-raytrace PUBLIC include/${CMAKE_PROJECT_NAME})
target_compile_features(mpi-raytrace PUBLIC cxx_std_23)
target_link_libraries(mpi-raytrace PUBLIC dependencies)

add_subdirectory(src)