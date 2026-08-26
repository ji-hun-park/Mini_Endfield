cmake_minimum_required(VERSION 3.15)
project(MiniEndfieldVulkanPlugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Plugin Source Files
set(SRC_FILES
    Src/PluginAPI.cpp
    Src/VulkanBackend.cpp
)

set(HEADER_FILES
    Src/PluginAPI.h
    Src/VulkanBackend.h
    Src/RenderGraph.h
    Src/SortKey.h
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
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../Assets/Plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../Assets/Plugins"
)

