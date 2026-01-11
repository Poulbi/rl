#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

in v3 pos;

out v3 Color;
out v2 LocalPos;
out v2 ButtonMin;
out v2 ButtonMax;

uniform v3 color;
uniform v2 buttonMin;
uniform v2 buttonMax;

void main()
{
    Color = color;
    ButtonMin = buttonMin;
    ButtonMax = buttonMax;
    
    LocalPos = pos.xy;
    
    gl_Position = v4(pos, 1.0f);
}