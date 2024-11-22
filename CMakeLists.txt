cmake_minimum_required(VERSION 3.22.1)

project(mpi-raytrace CXX)

find_package(MPI REQUIRED CXX)

include(cmake/CPM.cmake)

set(CMAKE_PROJECT_CLAPACK_INCLUDE_BEFORE ${CMAKE_CURRENT_LIST_DIR}/cmake/FixCLAPACK.cmake)
CPMAddPackage(
  NAME LAPACK
  VERSION 3.2.1
  URL http://www.netlib.org/clapack/clapack-3.2.1-CMAKE.tgz
)

if(NOT LAPACK_ADDED)
  message(FATAL_ERROR "LAPACK not added")
else()
  target_include_directories(f2c INTERFACE "${LAPACK_SOURCE_DIR}/INCLUDE")
  target_include_directories(lapack INTERFACE "${LAPACK_SOURCE_DIR}/INCLUDE")
endif()

CPMAddPackage(
  NAME blaze
  GIT_REPOSITORY https://bitbucket.org/blaze-lib/blaze.git
  VERSION 3.8.2
  OPTIONS
    "BLAZE_SMP_THREADS C++11"
    "BLAZE_BLAS_IS_64BIT 1"
)
if(NOT blaze_ADDED)
  message(FATAL_ERROR "blaze not added")
endif()

add_subdirectory(vendor/fpng)

add_library(dependencies INTERFACE)
target_link_libraries(dependencies INTERFACE MPI::MPI_CXX fpng blaze)
# target_include_directories(dependencies INTERFACE "${fpng_SOURCE_DIR}")

add_executable(mpi-raytrace)
target_include_directories(mpi-raytrace PUBLIC include/${CMAKE_PROJECT_NAME})
target_compile_features(mpi-raytrace PUBLIC cxx_std_23)
target_link_libraries(mpi-raytrace PUBLIC dependencies)

add_subdirectory(src)