#Licensed to the Apache Software Foundation (ASF) under one
#or more contributor license agreements.  See the NOTICE file
#distributed with this work for additional information
#regarding copyright ownership.  The ASF licenses this file
#to you under the Apache License, Version 2.0 (the
#"License"); you may not use this file except in compliance
#with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing,
#software distributed under the License is distributed on an
#"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#KIND, either express or implied.  See the License for the
#specific language governing permissions and limitations
#under the License.  

cmake_minimum_required(VERSION 3.15)

project(cppguard-distribution)

set(CMAKE_CXX_STANDARD 14)

if (WIN32)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

include(CTest)

add_subdirectory(cppguard)
add_subdirectory(cppguard_tests)

if (WIN32)
add_subdirectory(quickstart_example)
add_subdirectory(cppguard_tests_static_lib)
add_subdirectory(cppguard_tests_dyn_lib)
endif()







