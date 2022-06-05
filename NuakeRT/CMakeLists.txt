add_compile_definitions(GLFW_INCLUDE_NONE)

add_executable(NuakeRT main.cpp)

target_include_directories(NuakeRT PUBLIC ../Dependencies/NuakeRenderer)
target_compile_features(NuakeRT PRIVATE cxx_std_17)
target_link_libraries(NuakeRT PUBLIC NuakeRenderer)