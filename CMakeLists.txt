# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ route_guide example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building route_guide.

cmake_minimum_required(VERSION 3.16)

project(ZkpAuth C CXX)

# Find dependencies
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Boost REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# --- gRPC/Protobuf code generation ---
# This part is kept in case you add .proto files later.
# If you don't use gRPC for now, you can comment this section out.

# Proto file
get_filename_component(zkp_auth_proto "${CMAKE_CURRENT_SOURCE_DIR}/protos/zkp_auth.proto" ABSOLUTE)
get_filename_component(zkp_auth_proto_path "${zkp_auth_proto}" PATH)

# Generated sources
set(zkp_auth_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/zkp_auth.pb.cc")
set(zkp_auth_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/zkp_auth.pb.h")
set(zkp_auth_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/zkp_auth.grpc.pb.cc")
set(zkp_auth_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/zkp_auth.grpc.pb.h")

# Check if the proto file exists before creating the command
if(EXISTS "${zkp_auth_proto}")
  add_custom_command(
        OUTPUT "${zkp_auth_proto_srcs}" "${zkp_auth_proto_hdrs}" "${zkp_auth_grpc_srcs}" "${zkp_auth_grpc_hdrs}"
        COMMAND protobuf::protoc
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
          --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
          -I "${zkp_auth_proto_path}"
          --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
          "${zkp_auth_proto}"
        DEPENDS "${zkp_auth_proto}")

  # Library for generated proto code
  add_library(zkp_auth_grpc_proto
    ${zkp_auth_grpc_srcs}
    ${zkp_auth_grpc_hdrs}
    ${zkp_auth_proto_srcs}
    ${zkp_auth_proto_hdrs})
  target_include_directories(zkp_auth_grpc_proto
    PUBLIC
      ${CMAKE_CURRENT_BINARY_DIR}
  )
  target_link_libraries(zkp_auth_grpc_proto
    gRPC::grpc++
    protobuf::libprotobuf)
endif()

# --- Your Application Executable ---
add_executable(zkp_app main.cpp auth_server.cpp auth_service_impl.cpp)

target_link_libraries(zkp_app
  PRIVATE zkp_auth_grpc_proto
  GTest::gtest
  GTest::gmock
  Boost::boost
  )

# --- Your Test Executable ---
enable_testing()
add_executable(zkp_test chaum_pedersen_test.cpp)
target_link_libraries(zkp_test
  GTest::gtest
  GTest::gtest_main
  GTest::gmock
  Boost::boost)

add_test(NAME zkp_test COMMAND zkp_test)