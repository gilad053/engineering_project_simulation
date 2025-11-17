cmake_minimum_required(VERSION 3.10)
project(many_core_simulator)

# Set C++17 standard requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Define executable target linking all source files
add_executable(many_core_simulator 
    main.cpp
    src/Config.cpp
    src/TaskGraph.cpp
    src/Scheduler.cpp
    src/Core.cpp
    src/DTCM.cpp
    src/Cache.cpp
    src/MemoryBank.cpp
    src/Interconnect.cpp
    src/MainMemory.cpp
    src/MemorySystem.cpp
    src/StatsCollector.cpp
    src/Simulator.cpp
)

# Configure output directory for binary
set_target_properties(many_core_simulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
