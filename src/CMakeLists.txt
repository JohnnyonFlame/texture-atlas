# Import external SDL2 libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

add_executable(example 
    "example/main.cpp" 
    "example/textures.cpp"
    "example/mesh.cpp"
    "example/renderer.cpp"
    "example/program.cpp"
    "texture_atlas.c")

set_property(TARGET example PROPERTY CXX_STANDARD 17)
set_property(TARGET example PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_definitions(example
    PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD=1
)

target_include_directories(example PUBLIC external 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/example
    ${SDL2_INCLUDE_DIRS} 
    ${ASSIMP_INCLUDE_DIRS})

target_link_libraries(example 
    ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} 
    assimp
    glm
    "Glad" 
    "ImGui")
    
# Don't supress stdout/stderr on windows, if on vscode you might want to set
# <"externalConsole": true> on your launch.json files.
if(MSVC)
    target_link_options(example PRIVATE /SUBSYSTEM:CONSOLE)
elseif(MINGW)
    target_link_options(example PRIVATE -mconsole)
endif()