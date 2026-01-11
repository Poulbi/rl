#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

in v2 TexCoord;

out v4 FragColor;

uniform sampler2D Texture;

void main()
{
    FragColor = texture(Texture, TexCoord);
}