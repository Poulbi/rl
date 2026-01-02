#version 330 core

in vec3 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D TexKitten;
uniform sampler2D TexPuppy;

void main()
{
    vec4 ColKitten = texture(TexKitten, TexCoord);
    vec4 ColPuppy = texture(TexPuppy, TexCoord);
    FragColor = mix(ColKitten, ColPuppy, 0.5f); 
}