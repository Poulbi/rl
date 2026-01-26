typedef struct app_state app_state;
struct app_state
{
    v3 Angle;
    v3 Offset;
};

typedef s32 face[4][3];
#define New(type, Name, Array, Count) type *Name = Array + Count; Count += 1;

//- Mine 
internal str8 
GetAsset(android_app *App, char *Path)
{
    str8 Result = {0};
    
    AAsset *File = AAssetManager_open(App->activity->assetManager, Path, AASSET_MODE_BUFFER);
	if(File)
    {
        Result.Size = AAsset_getLength(File);
    }
    if(Result.Size)
    {
        Result.Data = (u8 *)AAsset_getBuffer(File);
    }
    
    return Result;
}

#define GLErrorInfo(Name) GLErrorInfo_(Name, __FILE__, __LINE__)
internal void
GLErrorInfo_(char *Name, char *FileName, s32 Line)
{
    s32 ErrorCode = glGetError();
    
    if(ErrorCode != GL_NO_ERROR)
    {
        
        char *String = 0;
        switch(ErrorCode)
        {
            case GL_INVALID_ENUM:                  String = "Invalid Enum"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: String = "Invalid Framebuffer Operation"; break; 
            case GL_INVALID_INDEX:                 String = "Invalid Index"; break; 
            case GL_INVALID_OPERATION:             String = "Invalid Operation"; break; 
            case GL_INVALID_VALUE:                 String = "Invalid Value"; break; 
            default:                               String = "Unknown Error"; break;
        }
        
        Log(ERROR_FMT " %s(%d): %s\n", FileName, Line, Name, ErrorCode, String);
    }
}

internal void 
GLErrorStatus(gl_handle Handle, str8 InfoLog, b32 IsShader)
{
    b32 Succeeded;
    
    if(IsShader)
    {
        glGetShaderiv(Handle, GL_COMPILE_STATUS, &Succeeded);
        glGetShaderInfoLog(Handle, (GLsizei)InfoLog.Size, NULL, (char *)InfoLog.Data);
    }
    else
    {
        glGetProgramiv(Handle, GL_LINK_STATUS, &Succeeded);
        glGetProgramInfoLog(Handle, (GLsizei)InfoLog.Size, NULL, (char *)InfoLog.Data);
    }
    
    if(!Succeeded)
    {
        Log("ERROR: %s\n", (char *)InfoLog.Data);
    }
    else
    {
        Log("%s compile succeded.\n", (IsShader) ? "Shader" : "Program");
    }
}

internal char *
Str8ToCString(arena *Arena, str8 In)
{
    char *Result = PushArray(Arena, char, In.Size + 1);
    memcpy(Result, In.Data, In.Size);
    Result[In.Size] = 0;
    
    return Result;
}

internal gl_handle
CompileShaderFromAsset(android_app *App, arena *Arena, str8 InfoLog, char *AssetPath, s32 Type)
{
    gl_handle Shader = glCreateShader(Type);
    str8 Source = GetAsset(App, AssetPath);
    
    if(Source.Size)
    {
        char *Data = Str8ToCString(Arena, Source);
        glShaderSource(Shader, 1, (GLchar *const *)&Data, NULL);
        glCompileShader(Shader);
        GLErrorStatus(Shader, InfoLog, true);
    }
    
    return Shader;
}


typedef struct load_obj_result load_obj_result;
struct load_obj_result
{
    umm Count;
    v3 *Vertices;
    v2 *TexCoords;
    v3 *Normals;
};

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
ParseFloat(str8 Inner, umm *Cursor)
{
    str8 In = S8From(Inner, *Cursor);
    
    b32 Negative = false;
    f32 Value = 0;
    
    umm At = 0;
    
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

internal load_obj_result
LoadObj(arena *Arena, android_app *AndroidApp, str8 File)
{
    load_obj_result Result = {0};
    
    str8 In = File;
    
    s32 InVerticesCount = 0;
    s32 InTexCoordsCount = 0;
    s32 InNormalsCount = 0;
    s32 InFacesCount = 0;
    v3 *InVertices = PushArray(Arena, v3, In.Size);
    v2 *InTexCoords = PushArray(Arena, v2, In.Size);
    v3 *InNormals = PushArray(Arena, v3, In.Size);
    face *InFaces = PushArray(Arena, face, In.Size);
    
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
                    New(v3, Vertex, InVertices, InVerticesCount);
                    Vertex->X = ParseFloat(In, &At);
                    Vertex->Y = ParseFloat(In, &At);
                    Vertex->Z = ParseFloat(In, &At);
                }
                else if(S8Match(Keyword, S8("vt"), false))
                {
                    New(v2, TexCoord, InTexCoords, InTexCoordsCount);
                    TexCoord->X = ParseFloat(In, &At);
                    // NOTE(luca): When looking at the texture in renderdoc I noticed it was flipped.
                    TexCoord->Y = 1.0f - ParseFloat(In, &At);
                }
                else if(S8Match(Keyword, S8("vn"), false))
                {
                    New(v3, Normal, InNormals, InNormalsCount);
                    Normal->X = ParseFloat(In, &At);
                    Normal->Y = ParseFloat(In, &At);
                    Normal->Z = ParseFloat(In, &At);
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
                            // NOTE(luca): Index start at 1 in obj file
                            (*InFace)[Y][X] = Value - 1;
                            
                            if(In.Data[At] == '/' || In.Data[At] == ' ') At += 1;
                        }
                        if(In.Data[At] == '\n') break;
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
    
    // Convert faces to triangles.
    {        
        // TODO(luca): This should depend on the number of v3es per face.  Blockbench always produces 4 so for now, we create a quad out of two triangles
        s32 Indices[] = {0, 1, 2, 0, 2, 3};
        s32 IndicesCount = ArrayCount(Indices);
        
        umm Count = InFacesCount*IndicesCount;
        v3 *Vertices = PushArray(Arena, v3, Count);
        v2 *TexCoords = PushArray(Arena, v2, Count);
        v3 *Normals = PushArray(Arena, v3, Count);
        
        for EachIndex(FaceIdx, InFacesCount)
        {
            for EachIndex(Idx, IndicesCount)
            {
                s32 Index = Indices[Idx];
                
                s32 *FaceIndices = InFaces[FaceIdx][Index];
                s32 vIdx = FaceIndices[0];
                s32 vtIdx = FaceIndices[1];
                s32 vnIdx = FaceIndices[2];
                
                Assert(vIdx >= 0 && vIdx < InVerticesCount);
                Assert(vtIdx >= 0 && vtIdx < InTexCoordsCount);
                Assert(vnIdx >= 0 && vnIdx < InNormalsCount);
                
                umm Offset = IndicesCount*FaceIdx + Idx;
                Vertices[Offset] = InVertices[vIdx];
                TexCoords[Offset] = InTexCoords[vtIdx];
                Normals[Offset] = InNormals[vnIdx];
            }
        }
        
        Result.Count = Count;
        Result.Vertices = Vertices;
        Result.TexCoords = TexCoords;
        Result.Normals = Normals;
    }
    
    return Result;
}

UPDATE_AND_RENDER(UpdateAndRender)
{
    b32 ShouldQuit = false;
    
    if(!Memory->Initialized)
    {
        Memory->AppState = PushStruct(PermanentArena, app_state);
        Memory->Initialized = true;
    }
    
    app_state *App = (app_state *)Memory->AppState;
    
    glViewport(0, 0, Buffer->Width, Buffer->Height);
    
    f32 Red = (f32)Input->MouseY/(f32)Buffer->Height;
    glClearColor(Red, Red, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    return ShouldQuit;
}
