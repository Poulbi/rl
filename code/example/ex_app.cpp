#include "base/base.h"
#include "ex_platform.h"

#define GLAD_GL_IMPLEMENTATION
#include "lib/gl_core.h"

typedef unsigned int gl_handle;

typedef struct vertex vertex;
struct vertex
{
    f32 X;
    f32 Y;
    f32 Z;
};

typedef struct tex_coord tex_coord;
struct tex_coord
{
    f32 X;
    f32 Y;
};

typedef vertex normal;

typedef s32 face[4][3];

#define New(type, Name, Array, Count) type *Name = Array + Count; Count += 1;

//~ Constants

// AA BB GG RR

#define ColorText          0xff87bfcf
#define ColorButtonText    0xFFFBFDFE
#define ColorPoint         0xFF00FFFF
#define ColorCursor        0xFFFF0000
#define ColorCursorPressed ColorPoint
#define ColorButton        0xFF0172AD
#define ColorButtonHovered 0xFF017FC0
#define ColorButtonPressed 0xFF0987C8
#define ColorBackground    0xFF13171F
#define ColorMapBackground 0xFF3A4151

#define HexToRGBV3(Hex) ((f32)((Hex >> 8*2) & 0xFF)/255.0f), \
((f32)((Hex >> 8*1) & 0xFF)/255.0f), \
((f32)((Hex >> 8*0) & 0xFF)/255.0f)


//~ Math
internal vertex 
Rotate(vertex V, f32 Angle)
{
    vertex Result = {};
    
    f32 C = cosf(Angle);
    f32 S = sinf(Angle);
    
    f32 X = V.X;
    f32 Y = V.Y;
    f32 Z = V.Z;
    
    Result.X = X*C - Z*S;
    Result.Y = Y;
    Result.Z = X*S + Z*C;
    
    return Result;
}

//~ GLAD
void GLADNullPreCallback(const char *name, GLADapiproc apiproc, int len_args, ...) {}

void GLADNullPostCallback(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...) {}

void GLADDisableCallbacks()
{
    _pre_call_gl_callback = GLADNullPreCallback;
    _post_call_gl_callback = GLADNullPostCallback;
}

void GLADEnableCallbacks()
{
    _pre_call_gl_callback = _pre_call_gl_callback_default;
    _post_call_gl_callback = _post_call_gl_callback_default;
}


//~ Helpers
internal inline u32 *
PixelFromBuffer(app_offscreen_buffer *Buffer, s32 X, s32 Y)
{
    Assert(X >= 0 && X < Buffer->Width);
    Assert(Y >= 0 && Y < Buffer->Height);
    
    u32 *Pixel = (u32 *)(Buffer->Pixels + Y*Buffer->Pitch + X*Buffer->BytesPerPixel);
    return Pixel;
}

internal void 
BubbleSort(u32 Count, u32 *List)
{
    for EachIndex(Outer, Count)
    {            
        b32 IsArraySorted = true;
        for EachIndex(Inner, (Count - 1))
        {
            if(List[Inner] > List[Inner + 1])
            {
                Swap(List[Inner], List[Inner + 1]);
                IsArraySorted = false;
            }
        }
        if(IsArraySorted)
        {
            break;
        }
    }
}

internal void
GLErrorStatus(gl_handle Handle, b32 IsShader)
{
    b32 Success = true;
    
    char InfoLog[KB(2)] = {};
    if(IsShader)
    {
        glGetShaderiv(Handle, GL_COMPILE_STATUS, &Success);
        glGetShaderInfoLog(Handle, sizeof(InfoLog), NULL, InfoLog);
    }
    else
    {
        glGetProgramiv(Handle, GL_LINK_STATUS, &Success);
        glGetProgramInfoLog(Handle, sizeof(InfoLog), NULL, InfoLog);
    }
    
    if(!Success)
    {
        ErrorLog("%s", InfoLog);
    }
}

internal gl_handle
CompileShaderFromSource(str8 Source, s32 Type)
{
    gl_handle Handle = glCreateShader(Type);
    
    glShaderSource(Handle, 1, (char **)&Source.Data, NULL);
    glCompileShader(Handle);
    GLErrorStatus(Handle, true);
    
    return Handle;
}


//~ Sorting
internal void 
MergeSortRecursive(u32 Count, u32 *First, u32 *Out)
{
    if(Count == 1)
    {
        // Nothing to be done.
    }
    else if(Count == 2)
    {
        if(First[0] > First[1]) 
        {
            Swap(First[0], First[1]);
        }
    }
    else
    {
        u32 Half0 = Count/2;
        u32 Half1 = Count - Half0;
        
        u32 *InHalf0 = First;
        u32 *InHalf1 = First + Half0;
        
        MergeSortRecursive(Half0, InHalf0, Out);
        MergeSortRecursive(Half1, InHalf1, Out + Half0);
        
        u32 *Write = Out;
        u32 *ReadHalf0 = InHalf0;
        u32 *ReadHalf1 = InHalf1;
        u32 *WriteEnd = Write + Count;
        u32 *End = First + Count;
        
        while(Write != WriteEnd)
        {
            if(ReadHalf1 == End)
            {
                *Write++ = *ReadHalf0;
            }
            else if(ReadHalf0 == InHalf1)
            {
                *Write++ = *ReadHalf1++;
            }
            else if(*ReadHalf0 < *ReadHalf1)
            {
                *Write++ = *ReadHalf0++;
            }
            else
            {
                *Write++ = *ReadHalf1++;
            }
        }
        
        for EachIndex(Idx, Count)
        {
            Swap(First[Idx], Out[Idx]);
        }
    }
}

internal void 
MergeSortIterative(u32 Count, u32 *In, u32 *Temp)
{
    u32 PairSize = 2;
    
    b32 Toggle = false;
    b32 LastMerge = false;
    while(!LastMerge)
    {    
        u32 PairsCount = CeilIntegerDiv(Count, PairSize);
        u32 Sorted = Count;
        
        LastMerge = (PairSize > Count);
        
        for EachIndex(Idx, PairsCount)
        {
            u32 *Group = In + Idx*PairSize;
            u32 *Out = Temp + Idx*PairSize;
            
            u32 GroupSize = ((PairSize <= Sorted) ? PairSize: Sorted);
            Sorted -= PairSize;
            u32 Half = PairSize/2;
            
            if(Half > GroupSize)
            {
                // NOTE(luca): Leftover group, just copy the values over
                Half = GroupSize;
            }
            
            u32 Half0 = Half;
            u32 Half1 = GroupSize - Half0;
            u32 *InHalf0 = Group;
            u32 *InHalf1 = Group + Half0;
            
            u32 *ReadHalf0 = InHalf0;
            u32 *ReadHalf1 = InHalf1;
            u32 *ReadEnd = InHalf0 + GroupSize;
            u32 *Write = Out;
            u32 *WriteEnd = Out + GroupSize;
            
            while(Write != WriteEnd)
            {
                if(ReadHalf0 == InHalf1)
                {
                    *Write++ = *ReadHalf1++;
                }
                else if(ReadHalf1 == ReadEnd)
                {
                    *Write++ = *ReadHalf0++;
                }
                else if(*ReadHalf0 < *ReadHalf1)
                {
                    *Write++ = *ReadHalf0++;
                }
                else
                {
                    *Write++ = *ReadHalf1++;
                }
            }
        }
        
        PairSize *= 2;
        
        Toggle = !Toggle;
        
        Swap(Temp, In);
    }
    
    if(Toggle)
    {
        MemoryCopy(Temp, In, sizeof(u32)*Count);
        Swap(In, Temp);
    }
    
}

//~ Parsing

internal inline 
b32 IsDigit(u8 Char)
{
    b32 Result = (Char >= '0' && Char <= '9');
    return Result;
}

internal str8
ParseName(str8 In, umm *At)
{
    str8 Result = {};
    
    umm Start = *At;
    while(!(In.Data[*At] == ' ' || In.Data[*At] == '\n')) *At += 1;
    Result = S8FromTo(In, Start, *At);
    
    return Result;
}

internal f32
ParseFloat(str8 Inner, u64 *Cursor)
{
    str8 In = S8From(Inner, *Cursor);
    
    b32 Negative = false;
    f32 Value = 0;
    
    u64 At = 0;
    
    if(In.Data[At] == '-')
    {
        At += 1;
        Negative = true;
    }
    
    // Integer part
    while(At < In.Size && IsDigit(In.Data[At])) 
    {
        f32 Digit = (f32)(In.Data[At] - '0');
        Value = 10.0f*Value + Digit;
        At += 1;
    }
    
    if(In.Data[At] == '.')
    {
        At += 1;
        
        // Fractional part
        f32 Divider = 10;
        while(At < In.Size && IsDigit(In.Data[At]))
        {
            f32 Digit = (f32)(In.Data[At] - '0');
            
            Value += (Digit / Divider);
            Divider *= 10;
            
            At += 1;
        }
    }
    
    if(Negative)
    {
        Value = -Value;
    }
    
    while(At < In.Size && In.Data[At] == ' ') At += 1; 
    
    *Cursor += At;
    
    return Value;
}

C_LINKAGE 
UPDATE_AND_RENDER(UpdateAndRender)
{
    OS_profiler Profiler = OS_ProfileInit();
    
    ThreadContextSelect(Context);
    
#if RL_INTERNAL    
    GlobalDebuggerIsAttached = App->DebuggerAttached;
#endif
    
    local_persist s32 GLADVersion = 0;
    // Init
    {    
#if RL_INTERNAL    
        if(App->Initialized && App->Reloaded)
        {
            GLADVersion = gladLoaderLoadGL();
            App->Reloaded = false;
        }
#endif
        
        if(!App->Initialized)
        {
            RandomSeed(&App->Series, 0);
            GLADVersion = gladLoaderLoadGL();
            
            App->Initialized = true;
        }
        
#if !RL_INTERNAL    
        GLADDisableCallbacks();
#endif
    }
    OS_ProfileAndPrint("Init", &Profiler);
    
    //Input 
    for EachIndex(Idx, Input->Text.Count)
    {
        app_text_button Key = Input->Text.Buffer[Idx];
        
        if(Key.Codepoint == 'a') App->Offset = 0.0f;
    }
    
    s32 Count = 0;
    vertex *Vertices = 0;
    tex_coord *TexCoords = 0; 
    normal *Normals = 0;
    
    // Read .obj file
    {    
        char *FileName = "./data/ladida.obj";
        str8 In = OS_ReadEntireFileIntoMemory(FileName);
        
        s32 InVerticesCount = 0;
        s32 InTexCoordsCount = 0;
        s32 InNormalsCount = 0;
        s32 InFacesCount = 0;
        vertex *InVertices = PushArray(FrameArena, vertex, In.Size);
        tex_coord *InTexCoords = PushArray(FrameArena, tex_coord, In.Size);
        normal *InNormals = PushArray(FrameArena, normal, In.Size);
        face *InFaces = PushArray(FrameArena, face, In.Size);
        
        if(In.Size)
        {
            for EachIndex(At, In.Size)
            {
                umm Start = At;
                
                if(In.Data[At] != '#')
                {
                    while(At < In.Size && !(In.Data[At] == ' ' || In.Data[At] == '\n')) At += 1;
                }
                
                if(In.Data[At] == ' ')
                {                    
                    str8 Keyword = S8FromTo(In, Start, At);
                    At += 1; // skip space
                    
                    if(0) {}
                    else if(S8Match(Keyword, S8("o"), false))
                    {
                        str8 Name = ParseName(In, &At);
                    }
                    else if(S8Match(Keyword, S8("v"), false))
                    {
                        New(vertex, Vertex, InVertices, InVerticesCount);
                        Vertex->X = ParseFloat(In, &At);
                        Vertex->Y = ParseFloat(In, &At);
                        Vertex->Z = ParseFloat(In, &At);
                    }
                    else if(S8Match(Keyword, S8("vt"), false))
                    {
                        New(tex_coord, InTexCoord, InTexCoords, InTexCoordsCount);
                        InTexCoord->X = ParseFloat(In, &At);
                        InTexCoord->Y = ParseFloat(In, &At);
                    }
                    else if(S8Match(Keyword, S8("vn"), false))
                    {
                        New(normal, InNormal, InNormals, InNormalsCount);
                        InNormal->X = ParseFloat(In, &At);
                        InNormal->Y = ParseFloat(In, &At);
                        InNormal->Z = ParseFloat(In, &At);
                    }
                    else if(S8Match(Keyword, S8("f"), false))
                    {
                        New(face, InFace, InFaces, InFacesCount);
                        
                        for EachIndex(Y, 4)
                        {
                            for EachIndex(X, 3)
                            {
                                s32 Value = 0;
                                // Integer part
                                while(At < In.Size && IsDigit(In.Data[At])) 
                                {
                                    s32 Digit = (s32)(In.Data[At] - '0');
                                    Value = 10*Value + Digit;
                                    At += 1;
                                }
                                (*InFace)[Y][X] = Value;
                                
                                if(In.Data[At] == '/' || In.Data[At] == ' ') At += 1;
                            }
                        }
                        
                    }
                    else if(S8Match(Keyword, S8("mtllib"), false))
                    {
                        str8 Name = ParseName(In, &At);
                    }
                    else if(S8Match(Keyword, S8("usemtl"), false))
                    {
                        str8 Name = ParseName(In, &At);
                    }
                }
                
                while(At < In.Size && In.Data[At] != '\n') At += 1;
            }
        }
        else
        {
            // NOTE(luca): Should already be logged in platform layer.
        }
        
        Count = InFacesCount*4;
        Vertices = PushArray(FrameArena, vertex, Count);
        TexCoords = PushArray(FrameArena, tex_coord, Count);
        Normals = PushArray(FrameArena, normal, Count);
        
        for EachIndex(Idx, InFacesCount)
        {
            for EachIndex(Y, 4)
            {
                s32 *Face = InFaces[Idx][Y];
                s32 VertexIdx = Face[0];
                s32 TexCoordIdx = Face[1];
                s32 NormalIdx = Face[2];
                
                umm Offset = Y*4 + Idx;
                Vertices[Offset] = InVertices[VertexIdx - 1];
                TexCoords[Offset] = InTexCoords[TexCoordIdx - 1];
                Normals[Offset] = InNormals[NormalIdx - 1];
            }
        }
        OS_FreeFileMemory(In);
        OS_ProfileAndPrint("Obj read", &Profiler);
    }
    
    s32 Major, Minor;
    glGetIntegerv(GL_MAJOR_VERSION, &Major);
    glGetIntegerv(GL_MINOR_VERSION, &Minor);
    
    glViewport(0, 0, Buffer->Width, Buffer->Height);
    
    str8 VertexShaderSource = OS_ReadEntireFileIntoMemory("./code/example/vert.glsl");
    str8 FragmentShaderSource = OS_ReadEntireFileIntoMemory("./code/example/frag.glsl");
    gl_handle VertexShader = CompileShaderFromSource(VertexShaderSource, GL_VERTEX_SHADER);
    gl_handle FragmentShader = CompileShaderFromSource(FragmentShaderSource, GL_FRAGMENT_SHADER);
    gl_handle ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    GLErrorStatus(ShaderProgram, false);
    
    glDeleteShader(FragmentShader); 
    glDeleteShader(VertexShader);
    OS_FreeFileMemory(VertexShaderSource);
    OS_FreeFileMemory(FragmentShaderSource);
    
    
    
    glUseProgram(ShaderProgram);
    
    gl_handle VAO, VBO;
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO);  
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    vertex vertices[] = 
    {
#if 1
        // Cube
        {0.5f,  -0.5f,  0.5f}, // BR
        {-0.5f, -0.5f,  0.5f}, // BL
        {-0.5f,  0.5f,  0.5f}, // TL
        {0.5f,  0.5f,   0.5f}, // TR
        
        {0.5f,  -0.5f,  -0.5f},
        {-0.5f, -0.5f,  -0.5f},
        {-0.5f,  0.5f,  -0.5f}, 
        {0.5f,  0.5f,   -0.5f},
#elif 1
        // Rectangle out of two triangles
        {0.5f,  -0.5f,  0.5f},
        {-0.5f, -0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f}, 
        
        {0.5f,  -0.5f,  0.5f},
        {-0.5f, 0.5f,  0.5f},
        {0.5f,  0.5f,  0.5f}, 
#endif
    };
    
    GLuint faces[] =
    {
        0, 1, 2, 3,
        4, 5, 6, 7,
        1, 2, 6, 5,
        0, 3, 7, 4,
    };
    
    App->Offset += Input->dtForFrame;
    
    GLint PosAttrib = glGetAttribLocation(ShaderProgram, "pos");
    glEnableVertexAttribArray(PosAttrib);
    glVertexAttribPointer(PosAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    
    f32 Angle = Pi32 * App->Offset;
    
    vertex Color = {cosf(Angle*2.0f), 0.5f, 0.0f};
    
    GLint UAngle = glGetUniformLocation(ShaderProgram, "angle");
    GLint UColor = glGetUniformLocation(ShaderProgram, "color");
    glUniform1f(UAngle, Angle);
    glUniform3f(UColor, Color.X, Color.Y, Color.Z);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    b32 Fill = true;
    s32 Mode = (Fill) ? GL_FILL : GL_LINE;
    glPolygonMode(GL_FRONT_AND_BACK, Mode);;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(HexToRGBV3(ColorMapBackground), 0.0f);
    glPointSize(10.0f);
    
    for EachIndex(Idx, 4)
    {
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, faces + 4*Idx);
    }
    
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(ShaderProgram);
    
    OS_ProfileAndPrint("GL", &Profiler);
}