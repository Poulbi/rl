internal P_context 
P_ContextInit(arena *Arena, app_offscreen_buffer *Buffer, b32 *Running)
{
    P_context Result = {};

				NotImplemented;
    
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

internal void
P_LoadAppCode(app_code *Code, app_state *AppState, s64 *LastWriteTime)
{
	HMODULE Library = (HMODULE)Code->LibraryHandle;

	char *TempDLLPath = "app_temp.dll";
	CopyFile(Code->LibraryPath, TempDLLPath, FALSE);

	Library = LoadLibraryA(TempDLLPath);
	if(Library)
	{
		Code->UpdateAndRender = (app_update_and_render *)GetProcAddress(Code.TempDLLPath, "UpdateAndRender");
		if(Code->UpdateAndRender)
		{
			Code->Loaded = true;
			AppState->Reloaded = true;
			Code->LibraryHandle = (umm)Library;
			Log("\nLibrary reloaded.\n");
		}
		else
		{
			Code->Loaded = false;
			ErrorLog("Could not find UpdateAndRender.\n");
		}
	}
	else
	{
		Code->Loaded = false;
		ErrorLog("Could not open library.\n");
	}

	if(!Code->Loaded)
	{
		Code->UpdateAndRender = UpdateAndRenderStub;
	}
}
