#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

in v3 pos;
in v2 tex;

out v2 TexCoord;

void main()
{
    TexCoord = tex;
    gl_Position = v4(pos, 1.0f);
}