project(flvmToy)
cmake_minimum_required(VERSION 3.25)
add_executable(flvmToy flvm.cc MurmurHash2.cc)

target_include_directories(
  flvmToy
  PRIVATE
  ${PROJECT_SOURCE_DIR}/
)