#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

in v3 Color;
in v2 LocalPos;
in v2 ButtonMin;
in v2 ButtonMax;

out v4 FragColor;

// NOTE(luca): Center is at 0.0 between -1.0 and 1.0
f32 DistanceFromPoint(v2 Center, f32 Radius)
{
    v2 Q = abs(Center) - v2(1.0f) + Radius;
    return length(max(Q, 0.0)) - Radius;
}

void main()
{
    // LocalPos -> -1.0f, 1.0f;
    
    f32 MinX = ((ButtonMin.x*2.0f) - 1.0f);
    f32 MaxX = ((ButtonMax.x*2.0f) - 1.0f);
    f32 MinY = (1.0f - ButtonMax.y*2.0f);
    f32 MaxY = (1.0f - ButtonMin.y*2.0f);
    v2 Min = v2(MinX, MinY);
    v2 Max = v2(MaxX, MaxY);
    
    if(LocalPos.x >= Min.x && LocalPos.x < Max.x &&
       LocalPos.y >= Min.y && LocalPos.y < Max.y)
    {
        v2 Size = Max - Min;
        v2 Pos = (LocalPos - Min);
        
        v2 Bilateral = 2.0f*(Pos/Size) - 1.0f;
        
        f32 Distance = DistanceFromPoint(Bilateral, 0.2f);
        f32 Alpha = 1.0 - smoothstep(0.0, 0.01, Distance);
        
        if(Alpha > 0.0f)
        {    
            FragColor = v4(Color, Alpha);
        }
    }
    
}