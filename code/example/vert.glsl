#version 330 core

layout (location = 0) in vec3 pos;

uniform vec3 color;
uniform float angle;

out vec3 Color;

void main()
{
    Color = color;
    
#if 1    
    float x, y, z;
    // Rotate
    {    
        float c = cos(angle);
        float s = sin(angle);
        x = pos.x*c - pos.z*s;
        z = pos.x*s + pos.z*c;
        y = pos.y;
    }
    
    float depth = z + 2.0;
    gl_Position = vec4(x/depth, y/depth, z, 1.0);
#else
    gl_Position = vec4(pos, 1.0f);
#endif
    
}