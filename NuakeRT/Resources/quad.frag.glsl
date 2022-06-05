#version 460 core

in vec2 v_UV;
out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    //FragColor = vec4(v_UV.x, v_UV.y, 0.f, 1.f);
    FragColor = texture(u_Texture, v_UV);
}