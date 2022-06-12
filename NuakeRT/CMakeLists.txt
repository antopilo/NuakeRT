add_compile_definitions(GLFW_INCLUDE_NONE)

add_executable(NuakeRT main.cpp Application/Application.h Application/Application.cpp 
Application/Raytracer.h Application/Raytracer.cpp Application/Scene.h Application/Scene.cpp 
Application/IO/FileSystem.h Application/IO/FileSystem.cpp Application/IO/ImGuizmo.h 
Application/IO/ImGuizmo.cpp Application/IO/InputManager.h Application/IO/InputManager.cpp 
Application/IO/Logger.h Application/IO/Logger.cpp Application/IO/stb_image_write.h 
Application/IO/tinyfiledialogs.cpp Application/IO/tinyfiledialogs.h)

target_include_directories(NuakeRT PUBLIC ../Dependencies/NuakeRenderer)
target_compile_features(NuakeRT PRIVATE cxx_std_17)
target_link_libraries(NuakeRT PUBLIC NuakeRenderer)