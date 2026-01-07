internal P_context 
P_ContextInit(arena *Arena, app_offscreen_buffer *Buffer, b32 *Running)
{
    P_context Result = {};
    
    DebugBreak;
    
    HGLRC GLContext;
    
    HDC OwnDC = GetDC(WindowHandle);
    
    PIXELFORMATDESCRIPTOR PixelFormat = {};
    PixelFormat.nSize = sizeof(PixelFormat);
    PixelFormat.nVersion = 1;
    PixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    PixelFormat.iPixelType = PFD_TYPE_RGBA;
    PixelFormat.cColorBits = 32;
    PixelFormat.cDepthBits = 24;
    PixelFormat.cStencilBits = 8;
    PixelFormat.iLayerType = PFD_MAIN_PLANE;
    
    int ChosenFormat = ChoosePixelFormat(OwnDC, &PixelFormat);
    SetPixelFormat(OwnDC, ChosenFormat, &PixelFormat);
    
    GLContext = wglCreateContext(OwnDC);
    wglMakeCurrent(OwnDC, GLContext);
    
    return Result;
}

internal void      
P_UpdateImage(P_context Context, app_offscreen_buffer *Buffer)
{
    NotImplemented;
}

internal void      
P_ProcessMessages(P_context Context, app_input *Input, app_offscreen_buffer *Buffer, b32 *Running)
{
    NotImplemented; 
}
