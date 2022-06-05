#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;

out vec2 v_UV;

uniform mat4 u_Projection;
uniform mat4 u_Model;

void main()
{
    v_UV = UV;
    gl_Position = u_Projection * u_Model * vec4(Position, 1.0f);
}