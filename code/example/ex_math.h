/* date = January 11th 2026 11:03 pm */

#ifndef EX_MATH_H
#define EX_MATH_H

typedef struct rect rect;
struct rect
{
    v2 Min;
    v2 Max;
};

internal inline v2 
V2AddV2(v2 A, v2 B)
{
    v2 Result = {};
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return Result;
}

internal inline v2 
V2AddF32(v2 A, f32 B)
{
    v2 Result = {};
    Result.X = A.X + B;
    Result.Y = A.Y + B;
    return Result;
}

internal inline v2 
V2MulF32(v2 A, f32 B)
{
    v2 Result = {};
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return Result;
}

internal inline v2
V2SubV2(v2 A, v2 B)
{
    v2 Result = {};
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return Result;
}

internal inline v2
V2S32(s32 X, s32 Y)
{
    v2 Result = {};
    Result.X = (f32)X;
    Result.Y = (f32)Y;
    return Result;
}

internal inline v2
V2MulV2(v2 A, v2 B)
{
    v2 Result = {};
    Result.X = A.X * B.X;
    Result.Y = A.Y * B.Y;
    return Result;
}

internal inline v2 
V2(f32 A, f32 B) 
{
    v2 Result = {};
    Result.X = A;
    Result.Y = B;
    return Result;
}

internal inline v3 
V3(f32 X, f32 Y, f32 Z) 
{
    v3 Result = {};
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    return Result;
}

internal inline b32
InBounds(v2 A, v2 Min, v2 Max)
{
    b32 Result = !!((A.X >= Min.X && A.X < Max.X) &&
                    (A.Y >= Min.Y && A.Y < Max.Y));
    return Result;
}

// TODO(luca): Metadesk

#define ProvokingFuncs \
SET(V4, v4) \
SET(V3, v3) \
SET(V2, v2) \
SET(F32, f32)

#define SET(Name, type) \
internal inline void \
SetProvoking##Name(type *Quad, type Value) \
{ \
Quad[2] = Value; Quad[5] = Value; \
}
ProvokingFuncs
#undef SET

internal inline void
MakeQuadV2(v2 *Quad, v2 Min, v2 Max)
{
    Quad[0] = {Min.X, Min.Y}; // BL
    Quad[1] = {Max.X, Min.Y}; // BR
    Quad[2] = {Min.X, Max.Y}; // TL
    Quad[3] = {Min.X, Max.Y}; // TL
    Quad[4] = {Max.X, Max.Y}; // TR
    Quad[5] = {Max.X, Min.Y}; // BR
}

internal inline void
MakeQuadV3(v3 *Quad, v2 Min, v2 Max, f32 Z)
{
    Quad[0] = {Min.X, Min.Y, Z}; // BL
    Quad[1] = {Max.X, Min.Y, Z}; // BR
    Quad[2] = {Min.X, Max.Y, Z}; // TL
    Quad[3] = {Min.X, Max.Y, Z}; // TL
    Quad[4] = {Max.X, Max.Y, Z}; // TR
    Quad[5] = {Max.X, Min.Y, Z}; // BR
}

#define E e[_VecMathIdx]
#define V2Math for EachIndex(_VecMathIdx, 2)
#define V3Math for EachIndex(_VecMathIdx, 3)
/*
v3 A = ;
v3 B = ;
v3 C = ;
V3Math { C.e = A.e * B.e; } 
*/

#endif //EX_MATH_H
