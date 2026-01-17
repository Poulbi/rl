//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <byteswap.h>
#include <errno.h>
#include <fcntl.h>

#define BASE_NO_ENTRYPOINT 1
#include "base/base.h"

NO_WARNINGS_BEGIN

#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>

#include "rawdraw/os_generic.h"
#include "rawdraw/CNFGAndroid.h"
#define CNFA_IMPLEMENTATION
#include "cnfa/CNFA.h"
#define CNFG_IMPLEMENTATION
#define CNFG3D
#include "rawdraw/CNFG.h"

NO_WARNINGS_END

#include "rld_libs.h"

//~ Types
typedef GLuint gl_handle;
typedef struct android_app android_app;

//~ Macro's
#undef Log
#define Log printf

#define SHOW_DEBUG_INFO 0

//~ Globals 
// Sensors
global_variable ASensorManager *SensorManager;
global_variable const ASensor *AccelerometerSensor;
global_variable bool NoSensorForGyro;
global_variable ASensorEventQueue* AccelerometerEventQueue;
global_variable ALooper *Looper;
// Audio
global_variable const u32 SAMPLE_RATE = 44100;
global_variable const u16 SAMPLE_COUNT = 512;
global_variable u32 StreamOffset;
global_variable u16 AudioFrequency;
// Accumulator
global_variable float AccelerometerX;
global_variable float AccelerometerY;
global_variable float AccelerometerZ;
global_variable int AccelerometerSamples;
// Keys
global_variable int LastButtonX;
global_variable int LastButtonY;
global_variable int LastMotionX;
global_variable int LastMotionY;
global_variable int LastButtonId;
global_variable int LastMask;
global_variable int LastKey;
global_variable int LastKeyDown;

static int KeyboardUp;
u8 ButtonState[8];

volatile b32 GlobalRunning;
volatile int Suspended;

//~ External

extern android_app *gapp;

extern void AndroidDisplayKeyboard(int pShow);

//~ Functions

void SetupIMU()
{
	SensorManager = ASensorManager_getInstanceForPackage("gyroscope");
	AccelerometerSensor = ASensorManager_getDefaultSensor(SensorManager, ASENSOR_TYPE_GYROSCOPE);
	NoSensorForGyro = AccelerometerSensor == NULL;
	Looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	AccelerometerEventQueue = ASensorManager_createEventQueue(SensorManager, (ALooper*)&Looper, 0, 0, 0); //XXX??!?! This looks wrong.
	if(!NoSensorForGyro) 
    {
		ASensorEventQueue_enableSensor(AccelerometerEventQueue, AccelerometerSensor);
		Log("setEvent Rate: %d\n", ASensorEventQueue_setEventRate(AccelerometerEventQueue, AccelerometerSensor, 10000));
	}
}

void AccCheck()
{
	if(!NoSensorForGyro) 
    {
		ASensorEvent Event;
        
        while(true)
        {
            smm SensorStatus = ASensorEventQueue_getEvents(AccelerometerEventQueue, &Event, 1);
            if(SensorStatus > 0)
            {
                AccelerometerX = Event.vector.v[0];
                AccelerometerY = Event.vector.v[1];
                AccelerometerZ = Event.vector.v[2];
                AccelerometerSamples++;
            }
            else
            {
                break;
            }
        }
    }
}

void HandleKey(int Keycode, int IsDown)
{
	LastKey = Keycode;
	LastKeyDown = IsDown;
	if(Keycode == 10 && !IsDown) 
    { 
        KeyboardUp = 0; 
        AndroidDisplayKeyboard(KeyboardUp);
    }
    
	if(Keycode == 4) 
    { 
        AndroidSendToBack(1);
    } //Handle Physical Back Button.
}

void HandleButton(int X, int Y, int Button, int IsDown)
{
	ButtonState[Button] = IsDown;
	LastButtonId = Button;
	LastButtonX = X;
	LastButtonY = Y;
    
	if(IsDown) 
    { 
        KeyboardUp = !KeyboardUp; 
        //AndroidDisplayKeyboard(KeyboardUp);
    }
}

void HandleMotion(int X, int Y, int Mask)
{
	LastMask = Mask;
	LastMotionX = X;
	LastMotionY = Y;
}

// NOTE: writes the text to a file to path: storage/emulated/0/Android/data/org.yourorg.cnfgtest/files
void Logger(const char *Format, ...)
{
	const char* Path = AndroidGetExternalFilesDir();
	char Buffer[2048];
	snprintf(Buffer, sizeof(Buffer), "%s/log.txt", Path);
	FILE *File = fopen(Buffer, "w");
	if (File == NULL)
	{
		exit(1);
	}
    
	va_list Arguments;
	va_start(Arguments, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, Arguments);
	va_end(Arguments);	
    
	fprintf(File, "%s\n", Buffer);
    
	fclose(File);
}

int HandleDestroy()
{
	Log("Destroying\n");
	return 0;
}

void HandleSuspend()
{
	Suspended = 1;
}

void HandleResume()
{
	Suspended = 0;
}

void HandleThisWindowTermination()
{
	Suspended = 1;
}

void AudioCallback(struct CNFADriver *SoundDriver, short *Output, short *Input, int FramesPending, int FramesReceived)
{
	memset(Output, 0, FramesPending*sizeof(u16));
	
    if(!Suspended)
    {
        if(ButtonState[1]) // play audio only if ~touching with two fingers
        {
            AudioFrequency = 440;
            for EachIndexType(smm, Index, FramesPending)
            {
                s16 Sample = INT16_MAX *sin(AudioFrequency*(2*M_PI)*(StreamOffset+Index)/SAMPLE_RATE);
                Output[Index] = Sample;
            }
            StreamOffset += FramesPending;
        }
    }
}
void MakeNotification(const char *channelID, const char *channelName, const char *title, const char *message)
{
	static int id;
	id++;
    
	const struct JNINativeInterface *env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface *jnii = *jniiptr;
    
	jnii->AttachCurrentThread(jniiptr, &envptr, NULL);
	env = (*envptr);
    
	jstring channelIDStr = env->NewStringUTF(ENVCALL channelID);
	jstring channelNameStr = env->NewStringUTF(ENVCALL channelName);
    
	// Runs getSystemService(Context.NOTIFICATION_SERVICE).
	jclass NotificationManagerClass = env->FindClass(ENVCALL "android/app/NotificationManager");
	jclass activityClass = env->GetObjectClass(ENVCALL gapp->activity->clazz);
	jmethodID MethodGetSystemService = env->GetMethodID(ENVCALL activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
	jstring notificationServiceName = env->NewStringUTF(ENVCALL "notification");
	jobject notificationServiceObj = env->CallObjectMethod(ENVCALL gapp->activity->clazz, MethodGetSystemService, notificationServiceName);
    
	// create the Notification channel.
	jclass notificationChannelClass = env->FindClass(ENVCALL "android/app/NotificationChannel");
	jmethodID notificationChannelConstructorID = env->GetMethodID(ENVCALL notificationChannelClass, "<init>", "(Ljava/lang/String;Ljava/lang/CharSequence;I)V");
	jobject notificationChannelObj = env->NewObject(ENVCALL notificationChannelClass, notificationChannelConstructorID, channelIDStr, channelNameStr, 3); // IMPORTANCE_DEFAULT
	jmethodID createNotificationChannelID = env->GetMethodID(ENVCALL NotificationManagerClass, "createNotificationChannel", "(Landroid/app/NotificationChannel;)V");
	env->CallVoidMethod(ENVCALL notificationServiceObj, createNotificationChannelID, notificationChannelObj);
    
	env->DeleteLocalRef(ENVCALL channelNameStr);
	env->DeleteLocalRef(ENVCALL notificationChannelObj);
    
	// Create the Notification builder.
	jclass classBuilder = env->FindClass(ENVCALL "android/app/Notification$Builder");
	jstring titleStr = env->NewStringUTF(ENVCALL title);
	jstring messageStr = env->NewStringUTF(ENVCALL message);
	jmethodID eventConstructor = env->GetMethodID(ENVCALL classBuilder, "<init>", "(Landroid/content/Context;Ljava/lang/String;)V");
	jobject eventObj = env->NewObject(ENVCALL classBuilder, eventConstructor, gapp->activity->clazz, channelIDStr);
	jmethodID setContentTitleID = env->GetMethodID(ENVCALL classBuilder, "setContentTitle", "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;");
	jmethodID setContentTextID = env->GetMethodID(ENVCALL classBuilder, "setContentText", "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;");
	jmethodID setSmallIconID = env->GetMethodID(ENVCALL classBuilder, "setSmallIcon", "(I)Landroid/app/Notification$Builder;");
    
	// You could do things like setPriority, or setContentIntent if you want it to do something when you click it.
    
	env->CallObjectMethod(ENVCALL eventObj, setContentTitleID, titleStr);
	env->CallObjectMethod(ENVCALL eventObj, setContentTextID, messageStr);
	env->CallObjectMethod(ENVCALL eventObj, setSmallIconID, 17301504); // R.drawable.alert_dark_frame
    
	// eventObj.build()
	jmethodID buildID = env->GetMethodID(ENVCALL classBuilder, "build", "()Landroid/app/Notification;");
	jobject notification = env->CallObjectMethod(ENVCALL eventObj, buildID);
    
	// NotificationManager.notify(...)
	jmethodID notifyID = env->GetMethodID(ENVCALL NotificationManagerClass, "notify", "(ILandroid/app/Notification;)V");
	env->CallVoidMethod(ENVCALL notificationServiceObj, notifyID, id, notification);
    
	env->DeleteLocalRef(ENVCALL notification);
	env->DeleteLocalRef(ENVCALL titleStr);
	env->DeleteLocalRef(ENVCALL activityClass);
	env->DeleteLocalRef(ENVCALL messageStr);
	env->DeleteLocalRef(ENVCALL channelIDStr);
	env->DeleteLocalRef(ENVCALL NotificationManagerClass);
	env->DeleteLocalRef(ENVCALL notificationServiceObj);
	env->DeleteLocalRef(ENVCALL notificationServiceName);
}

//- Entrypoint 

#include "app.c"

int 
main(int ArgsCount, char *Args[])
{
    short ScreenWidth, ScreenHeight;
    u32 Frames = 0;
    u32 FrameIdx = 0;
    
    double ThisTime;
    double LastFPSTime = OGGetAbsoluteTime();
    
    Log("Starting Up");
    
    CNFGBGColor = 0x000040ff;
    CNFGSetupFullscreen("Test Bench", 0);
    HandleWindowTermination = HandleThisWindowTermination;
    
    SetupIMU();
    InitCNFAAndroid(AudioCallback, "A Name", SAMPLE_RATE, 0, 1, 0, SAMPLE_COUNT, 0, 0, 0);
    
    Log("#### Startup Complete ####\n");
    
    arena *PermanentArena = ArenaAlloc();
    arena *FrameArena = ArenaAlloc();
    
    umm FrameArenaBackPos = BeginScratch(FrameArena);
    
    android_app *AndroidApp = gapp;
    
#if !SHOW_DEBUG_INFO    
    gl_handle ShaderProgram; 
    {
        str8 InfoLog = {0};
        InfoLog.Size = KB(2);
        InfoLog.Data = PushArray(FrameArena, u8, InfoLog.Size);
        
        gl_handle VertexShader, FragmentShader;
        VertexShader = CompileShaderFromAsset(AndroidApp, FrameArena, InfoLog, 
                                              "vert.glsl", GL_VERTEX_SHADER); 
        FragmentShader = CompileShaderFromAsset(AndroidApp, FrameArena, InfoLog, 
                                                "frag.glsl", GL_FRAGMENT_SHADER); 
        
        ShaderProgram = glCreateProgram();
        glAttachShader(ShaderProgram, VertexShader);
        glAttachShader(ShaderProgram, FragmentShader);
        glLinkProgram(ShaderProgram);
        GLErrorStatus(ShaderProgram, InfoLog, false);
        glDeleteShader(FragmentShader); 
        glDeleteShader(VertexShader);
    }
    
    gl_handle VAO, VBO[2], Tex;
    gl_handle PosAttrib, TexAttrib, UOffset, UAngle, UColor;
    {    
        glGenVertexArrays(1, &VAO); 
        glGenBuffers(2, &VBO[0]);  
        glGenTextures(1, &Tex);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        
        PosAttrib = glGetAttribLocation(ShaderProgram, "pos");
        glEnableVertexAttribArray(PosAttrib);
        glVertexAttribPointer(PosAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(v3), 0);
        
        TexAttrib = glGetAttribLocation(ShaderProgram, "tex");
        glEnableVertexAttribArray(TexAttrib);
        glVertexAttribPointer(TexAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(v2), 0);
        
        UOffset = glGetUniformLocation(ShaderProgram, "offset");
        UAngle = glGetUniformLocation(ShaderProgram, "angle");
        UColor = glGetUniformLocation(ShaderProgram, "color");
    }
#endif
    
    //~ 
    str8 AssetText = GetAsset(AndroidApp, "asset.txt");
    
    load_obj_result Model = LoadObj(PermanentArena, gapp, "bonhomme.obj");
    
    app_state App = {0};
    App.Angle.Z = 3.0f;
    
    GlobalRunning = true;
    
    int PreviousButtonX = 0;
    int PreviousButtonY = 0;
    
    while(GlobalRunning)
    {
        EndScratch(FrameArena, FrameArenaBackPos);
        
        FrameIdx++;
        
#if 0        
        if(FrameIdx == 200)
        {
            MakeNotification("default", "rldroid alerts", "rldroid", "Hit frame two hundred\nNew Line");
        }
#endif
        
        CNFGHandleInput();
        AccCheck();
        
        if(!Suspended)
        {
            CNFGClearFrame();
            CNFGGetDimensions(&ScreenWidth, &ScreenHeight);
            CNFGFlushRender();
            
            // Input
            {
                local_persist b32 IsDragging = false;
                
                if(ButtonState[0])
                {
                    f32 Movement;
                    if(IsDragging)
                    {
                        Movement = ((f32)(LastMotionX - PreviousButtonX)/(f32)ScreenWidth);
                        PreviousButtonX = LastMotionX;
                    }
                    else
                    {
                        // No movement on first tap
                        Movement = 0.0f;
                        LastMotionX = LastButtonX;
                        PreviousButtonX = LastButtonX;
                    }
                    
                    App.Offset.X += Movement*4.0f;
                }
                
                // Update drag state
                if(!ButtonState[0])
                {
                    IsDragging = false;
                }
                
                if(ButtonState[0] && !IsDragging)
                {
                    IsDragging = true;
                }
                
            }
            
#if !SHOW_DEBUG_INFO            
            glBindVertexArray(VAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(v3)*Model.Count, Model.Vertices, GL_STATIC_DRAW);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(v2)*Model.Count, Model.TexCoords, GL_STATIC_DRAW);
            
            glUseProgram(ShaderProgram);
            
            glUniform2f(UAngle, App.Offset.X, App.Offset.Y);
            glUniform3f(UOffset, App.Angle.X, App.Angle.Y, App.Angle.Z);
            glUniform3f(UColor, 1.0f, 0.5f, 0.0f);
            
            // Load texture
            {            
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, Tex);
                
                local_persist int Width, Height, Components;
                local_persist u8 *Image = 0;
                if(!Image)
                {
                    str8 ImageFile = GetAsset(AndroidApp,"bonhomme.png");
                    Image = stbi_load_from_memory(ImageFile.Data, ImageFile.Size, &Width, &Height, &Components, 0);
                }
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
                glUniform1i(glGetUniformLocation(ShaderProgram, "Texture"), 0);
                
#if 0                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                // TODO(luca): Use mipmap
#endif
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if 0
                f32 Color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
                glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, Color);
#endif
            }
            
#if 0            
            b32 Fill = true;
            s32 Mode = (Fill) ? GL_FILL : GL_LINE;
            glPolygonMode(GL_FRONT_AND_BACK, Mode);
#endif
            
            glViewport(0, 0, ScreenWidth, ScreenHeight);
            
#if 0
            glEnable(GL_DEPTH_TEST);
            GLErrorInfo("glEnable");
#endif
            
            glClearColor(1.0f, 0.2f, 0.2f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glDrawArrays(GL_TRIANGLES, 0, Model.Count);
            GLErrorInfo("Check");
#endif
            
#if SHOW_DEBUG_INFO            
            // Debug Info
            {
                CNFGColor(0xFFFFFFFF);
                CNFGPenX = 0; CNFGPenY = 400;
                char *TextToDraw = PushArray(FrameArena, char, AssetText.Size + 1);
                memcpy(TextToDraw, AssetText.Data, AssetText.Size);
                TextToDraw[AssetText.Size] = 0;
                CNFGDrawText(TextToDraw, 15);
                
                CNFGColor(0xFFFFFFFF);
                CNFGPenX = 0; CNFGPenY = 480;
                char *StatusText = PushArray(FrameArena, char, 255);
                sprintf(StatusText, 
                        "%dx%d %d %d %d %d %d %d\n"
                        "%d %d\n"
                        "%5.2f %5.2f %5.2f %d\n"
                        "%.2f"
                        ,
                        ScreenWidth, ScreenHeight, LastButtonX, LastButtonY, LastMotionX, LastMotionY, LastKey, LastKeyDown, 
                        LastButtonId, LastMask, AccelerometerX, AccelerometerY, AccelerometerZ, AccelerometerSamples,
                        App.Offset.X);
                
                
                CNFGDrawText(StatusText, 10);
            }
#endif
            
            Frames++;
            CNFGSwapBuffers();
            
            ThisTime = OGGetAbsoluteTime();
            if(ThisTime > LastFPSTime + 1)
            {
                Log("FPS: %d\n", Frames);
                Frames = 0;
                LastFPSTime+=1;
            }
            
        }
        else
        { 
            usleep(50000);
        }
        
    }
    
    return 0;
}
