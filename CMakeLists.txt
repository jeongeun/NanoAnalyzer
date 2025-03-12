cmake_minimum_required(VERSION 3.16)
project(NanoAnalysis VERSION 1.0 LANGUAGES CXX)

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set install prefix
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find ROOT and required components
find_package(ROOT REQUIRED COMPONENTS Core Tree RIO Hist)
find_package(yaml-cpp REQUIRED)
find_package(correctionlib REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(ZLIB)

include_directories(${ROOT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Gather all source and header files
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")
file(GLOB INC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# Separate the main source file
set(MAIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cc")
list(REMOVE_ITEM SRC_FILES ${MAIN_SRC})  # Exclude Main.cc from the library

# Create a static library containing all source files except Main.cc
add_library(NanoAnalyzerLib STATIC ${SRC_FILES})
target_include_directories(NanoAnalyzerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
  NanoAnalyzerLib PUBLIC 
  ${ROOT_LIBRARIES}
  ${YAML_CPP_LIBRARIES}
  correctionlib
)

# Create the NanoAnalysis executable
add_executable(NanoAnalysis ${MAIN_SRC})
target_link_libraries(NanoAnalysis PRIVATE NanoAnalyzerLib)

# Install rules
install(TARGETS NanoAnalysis RUNTIME DESTINATION bin)
install(TARGETS NanoAnalyzerLib ARCHIVE DESTINATION lib)
install(FILES ${INC_FILES} DESTINATION include)

