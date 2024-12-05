cmake_minimum_required(VERSION 3.22.1)

project(mpi-raytrace CXX)

find_package(MPI REQUIRED CXX)

include(cmake/CPM.cmake)

set(BLA_SIZEOF_INTEGER 8)
CPMAddPackage(
  NAME LAPACK
  GITHUB_REPOSITORY OpenMathLib/OpenBLAS
  GIT_TAG v0.3.28
  OPTIONS
    "BUILD_TESTING OFF"
)
if(NOT LAPACK_ADDED)
  message(FATAL_ERROR "LAPACK not added")
endif()
find_package(LAPACK REQUIRED CXX)

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

CPMAddPackage(
  NAME cxxopts
  GITHUB_REPOSITORY jarro2783/cxxopts
  GIT_TAG v3.2.0
)
if(NOT cxxopts_ADDED)
  message(FATAL_ERROR "cxxopts not added")
endif()

# add_subdirectory(vendor/fpng)
add_subdirectory(vendor/quasi-random)

add_library(dependencies INTERFACE)
target_link_libraries(dependencies INTERFACE MPI::MPI_CXX blaze quasirand cxxopts)
# target_include_directories(dependencies INTERFACE "${fpng_SOURCE_DIR}")
target_compile_options(dependencies INTERFACE -Wno-everything)

add_executable(mpi-raytrace)
target_include_directories(mpi-raytrace PUBLIC include/${CMAKE_PROJECT_NAME})
target_compile_features(mpi-raytrace PUBLIC cxx_std_23)
target_link_libraries(mpi-raytrace PUBLIC dependencies)

if(NOT NDEBUG)
  target_compile_options(mpi-raytrace PRIVATE -fsanitize=address)
  target_link_options(mpi-raytrace PRIVATE -fsanitize=address)
  set_target_properties(mpi-raytrace PROPERTIES LINKER_TYPE LLD)
endif()

add_subdirectory(src)