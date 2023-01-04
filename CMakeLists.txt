cmake_minimum_required(VERSION 3.10)
Include(FetchContent)

# set the project name
project(Noarr VERSION 0.17.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF) # we do not want to allow extensions
set(CMAKE_CXX_STANDARD_REQUIRED True)

# download testing library Catch2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1)
FetchContent_MakeAvailable(Catch2)

# setup the test runner executable
file(GLOB_RECURSE tests_source_files CONFIGURE_DEPENDS
    "tests/*_test.hpp"
    "tests/*_test.cpp"
    "tests/*_test.cu"
)
add_executable(test-runner tests/main_test.cpp ${tests_source_files})

# ask compiler to print maximum warnings
if(CMAKE_CXX_COMPILER MATCHES ".*/nvcc")
  project(Noarr LANGUAGES CXX CUDA)

  target_compile_options(test-runner PRIVATE -Xcompiler -Wall,-Wextra,-pedantic)
else()
  project(Noarr LANGUAGES CXX)

  if(MSVC)
    target_compile_options(test-runner PRIVATE /W4)
  else()
    target_compile_options(test-runner PRIVATE -Wall -Wextra -pedantic)
  endif()
endif()

target_include_directories(test-runner PUBLIC include)
target_link_libraries(test-runner PRIVATE Catch2::Catch2)

# setup CTest integration
# DISABLED: because 1. it spits out test name warnings, 2. nobody uses it anyways
# would be used by the "ctest" command, but we use "./test-runner" instead
#
# file(
#     DOWNLOAD
#     "https://raw.githubusercontent.com/catchorg/Catch2/devel/extras/ParseAndAddCatchTests.cmake"
#     "ParseAndAddCatchTests.cmake")
# enable_testing()
# include("ParseAndAddCatchTests.cmake")
# ParseAndAddCatchTests(test-runner)
