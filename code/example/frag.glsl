#version 330 core

out vec4 FragColor;

in vec3 Color;

void main()
{
#if 0
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
#else
    FragColor = vec4(Color, 1.0f);
#endif
}