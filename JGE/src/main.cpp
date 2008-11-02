


#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h> 
#include <psppower.h>
#include <pspsdk.h> 
#include <pspaudiocodec.h> 
#include <pspaudio.h> 
#include <pspaudiolib.h>
#include <pspmpeg.h>
#include <malloc.h>
#include <string.h>


//#include <mikmod.h>

#include "../../JGE/include/JGE.h"
#include "../../JGE/include/JApp.h"
#include "../../JGE/include/JGameLauncher.h"
#include "../../JGE/include/JRenderer.h"

#ifndef JGEApp_Title
#define JGEApp_Title "JGE++"
#endif


#ifdef DEVHOOK
	
	PSP_MODULE_INFO(JGEApp_Title, 0, 1, 1);
	PSP_MAIN_THREAD_ATTR(0);
	PSP_HEAP_SIZE_KB(-256);

#else

	PSP_MODULE_INFO(JGEApp_Title, 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
//PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
#endif


int mikModThreadID = -1;
bool done = false;

JApp *game = NULL;
JGE *engine = NULL;

//------------------------------------------------------------------------------------------------
// Exit callback
int exit_callback(int arg1, int arg2, void *common)
{
	if (engine != NULL)
		engine->End();

	sceKernelExitGame();

	return 0;
}

//------------------------------------------------------------------------------------------------
// Power Callback
int power_callback(int unknown, int pwrflags, void *common)
{
	if ((pwrflags & (PSP_POWER_CB_POWER_SWITCH | PSP_POWER_CB_STANDBY)) > 0) 
	{
		// suspending
		if (engine != NULL)
			engine->Pause();

		//sceKernelDelayThread(1000000);
	}
	else if ((pwrflags & PSP_POWER_CB_RESUME_COMPLETE) > 0)	
	{
		sceKernelDelayThread(1500000);
		// resume complete
		if (engine != NULL)
			engine->Resume();
	}

//     /* check for power switch and suspending as one is manual and the other automatic */
//     if (pwrflags & PSP_POWER_CB_POWER_SWITCH || pwrflags & PSP_POWER_CB_SUSPENDING) 
// 	{
// 		// suspending
// 		if (engine != NULL)
// 			engine->Pause();
//     } 
// 	else if (pwrflags & PSP_POWER_CB_RESUMING) 
// 	{
// 		// resuming from suspend mode
// 
//     } 
// 	else if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE) 
// 	{
// 		// resume complete
// 		if (engine != NULL)
// 			engine->Resume();
//     } 
// 	else if (pwrflags & PSP_POWER_CB_STANDBY)
// 	{
// 		// entering standby mode
// 		if (engine != NULL)
// 			engine->Pause();
//     } 
// 	else 
// 	{
// 		// unhandled power event
//     }
//     sceDisplayWaitVblankStart();

	return 0;
}

//------------------------------------------------------------------------------------------------
// Callback thread
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

#ifdef DEVHOOK
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
#endif
	cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
    scePowerRegisterCallback(0, cbid);

	sceKernelSleepThreadCB();

	return 0;
}

//------------------------------------------------------------------------------------------------
// static int AudioChannelThread(SceSize args, void *argp)
// {
// 	while (!done)
// 	{
// 		MikMod_Update();
// 		// We have to sleep here to allow other threads a chance to process.
// 		// with no sleep this thread will take over when the output is disabled via MikMod_DisableOutput()
// 		// co-operative threading sucks bigtime...
// 		sceKernelDelayThread(1);
// 	}
// 	return (0);
// }

//------------------------------------------------------------------------------------------------
// Sets up the callback thread and returns its thread id
int SetupCallbacks(void)
{
	int thid = 0;

    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0) 
	{
		sceKernelStartThread(thid, 0, 0);
    }

    return thid;
} 



#ifdef DEVHOOK

#else
	//------------------------------------------------------------------------------------------------
	// Custom exception handler
	void MyExceptionHandler(PspDebugRegBlock *regs)
	{
		pspDebugScreenInit();

		pspDebugScreenSetBackColor(0x00FF0000);
		pspDebugScreenSetTextColor(0xFFFFFFFF);
		pspDebugScreenClear();

		pspDebugScreenPrintf("I regret to inform you your psp has just crashed\n");
		pspDebugScreenPrintf("Please contact Sony technical support for further information\n\n");
		pspDebugScreenPrintf("Exception Details:\n");
		pspDebugDumpException(regs);
		pspDebugScreenPrintf("\nBlame the 3rd party software, it cannot possibly be our fault!\n");

		sceKernelExitGame();
	}


	//------------------------------------------------------------------------------------------------
	// Sort of hack to install exception handler under USER THREAD
	__attribute__((constructor)) void handlerInit()
	{
		pspKernelSetKernelPC();

		pspSdkInstallNoDeviceCheckPatch(); 
		pspSdkInstallNoPlainModuleCheckPatch(); 
		pspSdkInstallKernelLoadModulePatch(); 

		pspDebugInstallErrorHandler(MyExceptionHandler);
	} 

#endif

//------------------------------------------------------------------------------------------------
// The main loop
int main()
{ 
	//scePowerSetClockFrequency(333, 333, 166); 

	SetupCallbacks(); 

//	pspDebugScreenInit();

// 	if ((mikModThreadID = sceKernelCreateThread("MikMod" , AudioChannelThread, 0x12,0x10000,0,0)) > 0)
// 	{
// 		sceKernelStartThread(mikModThreadID, 0 , 0);
// 	}

	engine = NULL;

	//engine->Init(0);
//
	JGameLauncher* launcher = new JGameLauncher();

	u32 flags = launcher->GetInitFlags();
	if ((flags&JINIT_FLAG_ENABLE3D)!=0)
		JRenderer::Set3DFlag(true);

	engine = JGE::GetInstance();

	game = launcher->GetGameApp();
	game->Create();


	engine->SetApp(game);
	engine->Run();
	
	//pspDebugScreenSetXY(10,10);
	//pspDebugScreenPrintf("Hello world!");

	game->Destroy();
	delete game;
	game = NULL;

	engine->SetApp(NULL);

	done = true;
	
//	if (mikModThreadID > 0)
//	{
//		////SceUInt timeout = 100000;
//		////sceKernelWaitThreadEnd(mikModThreadID, &timeout);
//		// not 100% sure if this is necessary after a clean exit, but just to make sure any resources are freed:
//		sceKernelDeleteThread(mikModThreadID);
//	}


// 	JFileSystem::Destroy();
// 	JParticleSystem::Destroy();
// 	JMotionSystem::Destroy();

//	launcher->CleanUp();
	delete launcher;

	JGE::Destroy();
	engine = NULL;


//	sceKernelSleepThread(); 

	sceKernelExitGame();
	
	return 0; 
} 
