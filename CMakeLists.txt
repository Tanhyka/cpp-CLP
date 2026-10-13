cmake_minimum_required(VERSION 3.20)
project(cpp_clp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(clp_core STATIC core/core.cpp)
add_library(clp::core ALIAS clp_core)
target_include_directories(clp_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>)
if(MSVC)
  target_compile_options(clp_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(clp_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(CLP_BUILD_CHECK "Build Core standalone rules-check executable" ON)
if(CLP_BUILD_CHECK)
  add_executable(clp_core_check core/core_check.cpp)
  target_link_libraries(clp_core_check PRIVATE clp::core)
endif()

# This Windows-only adapter is optional; it is not the rules implementation.
option(CLP_BUILD_RULES_SERVICE "Build Windows clp_core_json DLL service" OFF)
if(CLP_BUILD_RULES_SERVICE)
  if(NOT WIN32)
    message(FATAL_ERROR "Existing rules_service.cpp uses Windows __declspec(dllexport). Disable CLP_BUILD_RULES_SERVICE on non-Windows platforms.")
  endif()
  add_library(clp_rules_service SHARED service/rules_service.cpp)
  target_link_libraries(clp_rules_service PRIVATE clp::core)
endif()

include(CTest)
if(BUILD_TESTING AND CLP_BUILD_CHECK)
  find_package(Python3 COMPONENTS Interpreter)
  if(Python3_Interpreter_FOUND)
    add_test(NAME clp_core_smoke
             COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_core_smoke.py" "$<TARGET_FILE:clp_core_check>")
  else()
    message(WARNING "Python interpreter not found: cannot run smoke test")
  endif()
endif()
