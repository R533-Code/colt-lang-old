cmake_minimum_required(VERSION 3.4)
# C++20 is required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Colt-Lang VERSION 0.0.1.8 LANGUAGES CXX)

# Set default build type to Release
set(CMAKE_BUILD_TYPE "Release")

# Save all the files to compile
file(GLOB_RECURSE ColtHeaders "src/*.h")
file(GLOB_RECURSE ColtUnits "src/*.cpp")
file(GLOB_RECURSE ColtGithubActions ".github/workflows/*.yml")
file(GLOB ColtDesignFiles "resources/*.md")
# Save test files
file(GLOB_RECURSE ColtASTTestsPath "resources/tests/ast/*.ct")
file(GLOB_RECURSE ColtLexerTestsPath "resources/tests/lexer/*.ct")

# Name of the compiler executable
set(COLT_EXECUTABLE_NAME colt)

# Create the compiler executable
if (WIN32)
  add_executable(${COLT_EXECUTABLE_NAME}
    ${ColtHeaders} ${ColtUnits} ${ColtASTTestsPath} ${ColtLexerTestsPath}
    "resources/icon/colt.rc"
    ${ColtGithubActions} ${ColtDesignFiles}
  )
else()
  add_executable(${COLT_EXECUTABLE_NAME}
    ${ColtHeaders} ${ColtUnits} ${ColtASTTestsPath} ${ColtLexerTestsPath}
    ${ColtGithubActions} ${ColtDesignFiles}
  )
endif()

# Add precompiled header
target_precompile_headers(${COLT_EXECUTABLE_NAME} PUBLIC 
  "$<$<COMPILE_LANGUAGE:CXX>:${PROJECT_SOURCE_DIR}/src/util/common/colt_pch.h>")

# Define COLT_DEBUG_BUILD for debug config
target_compile_definitions(
  ${COLT_EXECUTABLE_NAME} PRIVATE $<$<CONFIG:Debug>:COLT_DEBUG> $<$<CONFIG:Debug>:COLT_DEBUG_BUILD> _CRT_SECURE_NO_WARNINGS
)

if (MSVC)
  # The executable is the startup project in Visual Studio
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${COLT_EXECUTABLE_NAME})
  
  # add_compile_options("/MT" "/EHsc" "/Oy-" "/Ob0")

  target_compile_options(
      ${COLT_EXECUTABLE_NAME} PUBLIC 
      "/external:anglebrackets"
      "/external:W0"
      "/Zc:preprocessor"
      "/Zi"
  )
  
  #target_compile_options(${COLT_EXECUTABLE_NAME} PRIVATE "$<$<CONFIG:Release>:/Zi>")
  #target_link_options(${COLT_EXECUTABLE_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
  #target_link_options(${COLT_EXECUTABLE_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
  #target_link_options(${COLT_EXECUTABLE_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
endif()

#########################################
# LLVM SETUP
#########################################

# option(COLT_NO_LLVM "Compile Colt without using LLVM" false)
# if (${COLT_NO_LLVM})
#   target_compile_definitions(
#     ${COLT_EXECUTABLE_NAME} PRIVATE "COLT_NO_LLVM"
#   )
# endif()

# message(STATUS "Setting up LLVM...")

# add_subdirectory(libraries/llvm-project/llvm)

# # Add includes of LLVM
# include_directories(${LLVM_INCLUDE_DIRS})
# separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
# add_definitions(${LLVM_DEFINITIONS_LIST})

# # Find the libraries that correspond to the LLVM components
# # that we wish to use
# llvm_map_components_to_libnames(llvm_libs
#   support analysis core executionengine
#   irreader passes orcjit instcombine
#   object mc interpreter asmparser asmprinter
#   nativecodegen mcjit codegen native selectiondag
#   X86AsmParser X86CodeGen X86Desc X86Disassembler
#   X86Info X86TargetMCA
# )

# # Link against LLVM libraries
# target_link_libraries(${COLT_EXECUTABLE_NAME} PUBLIC "${llvm_libs}")

# message(STATUS "Finished LLVM set up!")

#########################################
# LIBRARIES SETUP
#########################################

message(STATUS "Setting up libraries...")

# Add {fmt}
message(STATUS "Adding 'fmt'...")
add_subdirectory("${PROJECT_SOURCE_DIR}/libraries/fmt")
target_link_libraries(${COLT_EXECUTABLE_NAME} PUBLIC fmt::fmt)

# Add scnlib
message(STATUS "Adding 'scnlib'...")
add_subdirectory("${PROJECT_SOURCE_DIR}/libraries/scnlib")
target_link_libraries(${COLT_EXECUTABLE_NAME} PUBLIC scn::scn)

# Add dyncall and dynload
message(STATUS "Adding 'dyncall' and 'dynload'...")
add_subdirectory("${PROJECT_SOURCE_DIR}/libraries/dyncall")
target_link_libraries(${COLT_EXECUTABLE_NAME} PUBLIC dyncall_s)
target_link_libraries(${COLT_EXECUTABLE_NAME} PUBLIC dynload_s)


# Directories for '#include <...>'
target_include_directories(${COLT_EXECUTABLE_NAME} PUBLIC 
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/src/frontend"
  "${CMAKE_SOURCE_DIR}/src/backend"
  "${CMAKE_SOURCE_DIR}/src/util"
  SYSTEM # So no warning is shown
  "${CMAKE_SOURCE_DIR}/libraries/fmt/include"
  "${CMAKE_SOURCE_DIR}/libraries/scnlib/include"
  "${CMAKE_SOURCE_DIR}/libraries/dyncall/"
  #"${CMAKE_SOURCE_DIR}/libraries/llvm-project/llvm/include"
  #"${CMAKE_BINARY_DIR}/libraries/llvm-project/llvm/include"
)

#########################################
# COLT TESTS
#########################################

message(STATUS "Searching for tests...")
# Load tests
list(LENGTH ColtASTTestsPath testCountAST)
list(LENGTH ColtLexerTestsPath testCountLexer)
message(STATUS "Found " ${testCountAST} " AST tests and " ${testCountLexer} " Lexer tests.")

# For VS, group tests
source_group("Colt AST Tests" FILES ${ColtASTTestsPath})
source_group("Colt Lexer Tests" FILES ${ColtLexerTestsPath})
source_group("Github Actions" FILES ${ColtGithubActions})
source_group("Colt Design" FILES ${ColtDesignFiles})

# Testing for CTest
enable_testing()

option(ENUM_TESTS "Enumerates tests to run" OFF)

if (NOT ${testCountAST} EQUAL 0 AND ${ENUM_TESTS})
  message(STATUS "Enumerating tests...")
endif()

# Contains the additional arguments to pass to the executable when testing
# -C: No Color (so no weird behavior happens when regex matching)
# -nowait: Do not wait for user input before closing the app.
set(COLT_ADDITIONAL_ARGS -C -nowait)

# TESTING:
# Each file in resources/tests/ast that ends with .ct
# is a file to try to run.
# The first line of these files is a comment '//.*'
# If the second line is a comment, it must be the expected error count
# of the compilation (a positive integer).
# which gives the regex to test the output of interpreting
# the file against.
# foreach(testPath ${ColtASTTestsPath})
#   # Read all the lines of the file
#   file(STRINGS ${testPath} testLines)
#   # Store the first line
#   list(GET testLines 0 firstLine)
#   
#   # Get test name
#   get_filename_component(testName ${testPath} NAME_WE)
# 
#   if (NOT "${firstLine}" MATCHES "//.*")
#     message(SEND_ERROR "Test '${testName}.ct' should begin with a '//' followed by the Regex to check for!")
#   endif()
# 
#   # Pop the 2 first characters which are //
#   string(REPLACE "//" "" RegexTest ${firstLine})
#   # Strip new lines from end of line
#   string(REGEX REPLACE "\n$" "" RegexTest "${RegexTest}")
# 
#   if ("${RegexTest}" MATCHES "^`.*")
#     string(REGEX REPLACE "([][+.*()^;])" "\\\\\\1" RegexTest "${RegexTest}")
#     # Pop the first characters which is '`'
#     string(REPLACE "`" "" RegexTest ${RegexTest})
#   endif()
# 
#   # Get folder before test name
#   get_filename_component(testFolder_temp "${testPath}/.." ABSOLUTE)
#   
#   # Get position of last '/'
#   string(FIND "${testFolder_temp}" "/" testFolderNameBegin REVERSE)
#   # Increment past '/'
#   MATH(EXPR testFolderNameBegin "${testFolderNameBegin}+1")
#   # Obtain the last directory name by striping the part before the last '/'
#   string(SUBSTRING "${testFolder_temp}" "${testFolderNameBegin}" "-1" testFolderName)
#   # Replace any spaces in the directory name by an '_'
#   string(REPLACE " " "_" testFolderName "${testFolderName}")
#   
#   string(TOUPPER ${testName} testName)
#   string(TOUPPER ${testFolderName} testFolderName)
#   # Example of name: resources/tests/syntax/binary.ct -> SYNTAX_BINARY
#   set(testName "${testFolderName}_${testName}")
#   #string(REPLACE " " "_" testName "${testName}")
#   string(REGEX REPLACE "([^a-zA-Z0-9_])" "_" testName "${testName}")
#   string(REGEX REPLACE "(_+)" "_" testName "${testName}")
#   string(REGEX REPLACE "_$" "" testName ${testName})
# 
#   # Create test
#   add_test(NAME "${testName}" COMMAND ${COLT_EXECUTABLE_NAME} ${COLT_ADDITIONAL_ARGS} ${testPath})
#   set_property(TEST ${testName} PROPERTY PASS_REGULAR_EXPRESSION ${RegexTest})
#   set_property(TEST ${testName} PROPERTY TIMEOUT 5) # 5s
#   if (${ENUM_TESTS})
#     message("Created test '${testName}' of REGEX [${RegexTest}].")
#   endif()
# endforeach()

# TESTING:
foreach(testPath ${ColtLexerTestsPath})
  # Get test name
  get_filename_component(testName ${testPath} NAME_WE)
  string(TOUPPER ${testName} testName)

  # Create test
  add_test(NAME "${testName}" COMMAND ${COLT_EXECUTABLE_NAME} ${COLT_ADDITIONAL_ARGS} -run-tests "-test-lexer=${testPath}")
  set_property(TEST ${testName} PROPERTY PASS_REGULAR_EXPRESSION "Tested [0-9]+ features with 0")
  set_property(TEST ${testName} PROPERTY TIMEOUT 10) # 10s
endforeach()

if (NOT ${testCountAST} EQUAL 0 AND ${ENUM_TESTS})
  message(STATUS "Finished enumerating tests!")
endif()

#########################################
# DOXYGEN
#########################################

# DOXYGEN
option(BUILD_DOC "Build documentation" ON)

message(STATUS "Searching for Doxygen executable...")
# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
  message(STATUS "Found Doxygen executable.")
  # set input and output files
  set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/resources/doxygen/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_SOURCE_DIR}/docs/Doxyfile.out)

  message(STATUS "Configuring Doxygen input file...")
  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})

  add_custom_target(GENERATE_DOCS
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
else (DOXYGEN_FOUND)
  message(WARNING "Doxygen was not found! Doxygen need to be installed to generate the documentation!")
endif (DOXYGEN_FOUND)

#########################################
# COLT CONFIG
#########################################

message(STATUS "Configuring 'colt_config.h'...")

# Useful macros for knowing which compiler is going to compile the code
set(IMPL_COLT_CLANG 0)
set(IMPL_COLT_GNU 0)
set(IMPL_COLT_INTEL 0)
set(IMPL_COLT_MSVC 0)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # AppleClang or Clang
  set(IMPL_COLT_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(IMPL_COLT_GNU 1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(IMPL_COLT_INTEL 1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(IMPL_COLT_MSVC 1)
endif()

set(IMPL_COLT_OS_WINDOWS 0)
set(IMPL_COLT_OS_LINUX 0)
set(IMPL_COLT_OS_APPLE 0)

if(WIN32)
  set(IMPL_COLT_OS_WINDOWS 1)
  set(IMPL_COLT_OS_STRING "Windows")
elseif(APPLE)
  set(IMPL_COLT_OS_APPLE 1)
  set(IMPL_COLT_OS_STRING "Apple")
elseif(UNIX AND NOT APPLE) #LINUX
  set(IMPL_COLT_OS_LINUX 1)
  set(IMPL_COLT_OS_STRING "Linux")
else()
  set(IMPL_COLT_OS_STRING "UNKNOWN")
endif()

configure_file("${CMAKE_SOURCE_DIR}/resources/cmake/cmake_colt_config.in"
  "${CMAKE_SOURCE_DIR}/src/util/common/colt_config.h")

message(STATUS "Finished configuring 'colt_config.h'!")