cmake_minimum_required(VERSION 3.15)
project(MiniEndfieldVulkanPlugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use local VulkanSDK
set(ENV{VULKAN_SDK} "${CMAKE_CURRENT_SOURCE_DIR}/../VulkanSDK/1.4.357.0")

# Find Vulkan
find_package(Vulkan REQUIRED)

# Plugin Source Files
set(SRC_FILES
    Src/PluginAPI.cpp
    Src/VulkanBackend.cpp
    Src/Benchmark.cpp
)

set(HEADER_FILES
    Src/PluginAPI.h
    Src/VulkanBackend.h
    Src/RenderGraph.h
    Src/SortKey.h
    Src/Benchmark.h
)

# Create Shared Library (.dll on Windows, .so on Linux)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${HEADER_FILES})

# Include Vulkan Headers
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})

# Link Vulkan Library
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Unity Plugin Defines
target_compile_definitions(${PROJECT_NAME} PRIVATE EXPORT_PLUGIN)

# Output to Unity project (Plugins folder)
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "MiniEndfieldVulkanPlugin_v12"
)

