cmake_minimum_required(VERSION 3.15)
project(ScypLib)

set(CMAKE_CXX_STANDARD 20)

add_library(ScypLib STATIC
    src/Audio.cpp
    src/EntityComponentSystem.cpp
    src/EventDispatcher.cpp
    src/Graphics.cpp
    src/Keyboard.cpp
    src/Mouse.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Window.cpp
    vendors/src/glad/glad.c
)

target_include_directories(ScypLib
    PUBLIC
        include
        vendors/include
        dependencies/GLFW/include
)

if(WIN32)
    target_compile_definitions(ScypLib PUBLIC _GLFW_WIN32)

    target_link_libraries(ScypLib
        PUBLIC
            opengl32
            gdi32
            user32
            shell32
    )
endif()

install(TARGETS ScypLib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)