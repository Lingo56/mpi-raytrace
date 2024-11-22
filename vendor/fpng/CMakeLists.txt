project(fpng LANGUAGES CXX)

add_library(fpng fpng.cpp auto_init.cpp)
target_include_directories(fpng PUBLIC include)

target_compile_features(fpng PUBLIC cxx_std_11)
target_compile_options(fpng PRIVATE -fvisibility=hidden -fPIC -fno-strict-aliasing)
target_compile_definitions(fpng PRIVATE -D_LARGEFILE64_SOURCE=1 -D_FILE_OFFSET_BITS=64)
