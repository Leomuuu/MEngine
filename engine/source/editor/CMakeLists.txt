set(EDITOR_DIR ${ENGINE_ROOT_DIR}/source/editor)

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

add_library(editor ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(editor  runtime)

target_link_libraries(editor  glfw glm tinyobjloader imgui)

find_package(Vulkan REQUIRED FATAL_ERROR)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(editor ${Vulkan_LIBRARIES})
