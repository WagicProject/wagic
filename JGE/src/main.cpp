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

#include <pspctrl.h>
#include <unistd.h>
#include <stdio.h>
#include <queue>


#include "../../JGE/include/JGE.h"
#include "../../JGE/include/JApp.h"
#include "../../JGE/include/JGameLauncher.h"
#include "../../JGE/include/JRenderer.h"
#include "../../JGE/include/JLogger.h"


#ifndef JGEApp_Title
#define JGEApp_Title "JGE++"
#endif


#ifdef DEVHOOK
PSP_MODULE_INFO(JGEApp_Title, 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
//256 is not enough for the network to correctly start,
// let's find an appropriate value the day JGE has working network
PSP_HEAP_SIZE_KB(-256);

#else

PSP_MODULE_INFO(JGEApp_Title, 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

#endif


int mikModThreadID = -1;
bool done = false;

JApp *game = NULL;
JGE *g_engine = NULL;

u32 gTickFrequency;

//------------------------------------------------------------------------------------------------
// Exit callback
int exit_callback(int arg1, int arg2, void *common)
{
    if (g_engine != NULL)
        g_engine->End();
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
        if (g_engine != NULL) g_engine->Pause();
    }
    else if ((pwrflags & PSP_POWER_CB_RESUME_COMPLETE) > 0)
    {
        sceKernelDelayThread(1500000);
        // resume complete
        if (g_engine != NULL)
            g_engine->Resume();
    }
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

// Sets up the callback thread and returns its thread id
int SetupCallbacks(void)
{
    int thid = 0;

    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);
    return thid;
}



#ifdef DEVHOOK
//code by sakya, crazyc, samuraiX
//http://forums.ps2dev.org/viewtopic.php?t=9591
PspDebugRegBlock exception_regs;

extern int _ftext;

static const char *codeTxt[32] =
{
    "Interrupt", "TLB modification", "TLB load/inst fetch", "TLB store",
    "Address load/inst fetch", "Address store", "Bus error (instr)",
    "Bus error (data)", "Syscall", "Breakpoint", "Reserved instruction",
    "Coprocessor unusable", "Arithmetic overflow", "Unknown 14",
    "Unknown 15", "Unknown 16", "Unknown 17", "Unknown 18", "Unknown 19",
    "Unknown 20", "Unknown 21", "Unknown 22", "Unknown 23", "Unknown 24",
    "Unknown 25", "Unknown 26", "Unknown 27", "Unknown 28", "Unknown 29",
    "Unknown 31"
};

static const unsigned char regName[32][5] =
{
    "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

void ExceptionHandler(PspDebugRegBlock * regs)
{
    int i;
    SceCtrlData pad;

    pspDebugScreenInit();
    pspDebugScreenSetBackColor(0x00FF0000);
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenClear();
    pspDebugScreenPrintf("Your PSP has just crashed!\n");
    pspDebugScreenPrintf("Exception details:\n\n");

    pspDebugScreenPrintf("Exception - %s\n", codeTxt[(regs->cause >> 2) & 31]);
    pspDebugScreenPrintf("EPC       - %08X / %s.text + %08X\n", (int)regs->epc, module_info.modname, (unsigned int)(regs->epc-(int)&_ftext));
    pspDebugScreenPrintf("Cause     - %08X\n", (int)regs->cause);
    pspDebugScreenPrintf("Status    - %08X\n", (int)regs->status);
    pspDebugScreenPrintf("BadVAddr  - %08X\n", (int)regs->badvaddr);
    for (i = 0; i < 32; i += 4) pspDebugScreenPrintf("%s:%08X %s:%08X %s:%08X %s:%08X\n", regName[i], (int)regs->r[i], regName[i+1], (int)regs->r[i+1], regName[i+2], (int)regs->r[i+2], regName[i+3], (int)regs->r[i+3]);

    sceKernelDelayThread(1000000);
    pspDebugScreenPrintf("\n\nPress X to dump information on file exception.log and quit");
    pspDebugScreenPrintf("\nPress O to quit");

    for (;;)
    {
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_CROSS)
        {
            FILE *log = fopen("exception.log", "w");
            if (log != NULL)
            {
                char testo[512];
                sprintf(testo, "Exception details:\n\n");
                fwrite(testo, 1, strlen(testo), log);
                sprintf(testo, "Exception - %s\n", codeTxt[(regs->cause >> 2) & 31]);
                fwrite(testo, 1, strlen(testo), log);
                sprintf(testo, "EPC       - %08X / %s.text + %08X\n", (int)regs->epc, module_info.modname, (unsigned int)(regs->epc-(int)&_ftext));
                fwrite(testo, 1, strlen(testo), log);
                sprintf(testo, "Cause     - %08X\n", (int)regs->cause);
                fwrite(testo, 1, strlen(testo), log);
                sprintf(testo, "Status    - %08X\n", (int)regs->status);
                fwrite(testo, 1, strlen(testo), log);
                sprintf(testo, "BadVAddr  - %08X\n", (int)regs->badvaddr);
                fwrite(testo, 1, strlen(testo), log);
                for (i = 0; i < 32; i += 4)
                {
                    sprintf(testo, "%s:%08X %s:%08X %s:%08X %s:%08X\n", regName[i], (int)regs->r[i], regName[i+1], (int)regs->r[i+1], regName[i+2], (int)regs->r[i+2], regName[i+3], (int)regs->r[i+3]);
                    fwrite(testo, 1, strlen(testo), log);
                }
                fclose(log);
            }
            break;
        }
        else if (pad.Buttons & PSP_CTRL_CIRCLE)
            break;
        sceKernelDelayThread(100000);
    }
    sceKernelExitGame();
}

void initExceptionHandler()
{
    SceKernelLMOption option;
    int args[2], fd, modid;

    memset(&option, 0, sizeof(option));
    option.size = sizeof(option);
    option.mpidtext = PSP_MEMORY_PARTITION_KERNEL;
    option.mpiddata = PSP_MEMORY_PARTITION_KERNEL;
    option.position = 0;
    option.access = 1;

    if ((modid = sceKernelLoadModule("exception.prx", 0, &option)) >= 0)
    {
        args[0] = (int)ExceptionHandler;
        args[1] = (int)&exception_regs;
        sceKernelStartModule(modid, 8, args, &fd, NULL);
    }
}
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

static const struct { LocalKeySym keysym; JButton keycode; } gDefaultBindings[] =
{
    { PSP_CTRL_START,    JGE_BTN_MENU },
    { PSP_CTRL_SELECT,   JGE_BTN_CTRL },
    { PSP_CTRL_RIGHT,    JGE_BTN_RIGHT },
    { PSP_CTRL_LEFT,     JGE_BTN_LEFT },
    { PSP_CTRL_UP,       JGE_BTN_UP },
    { PSP_CTRL_DOWN,     JGE_BTN_DOWN },
    { PSP_CTRL_CIRCLE,   JGE_BTN_OK },
    { PSP_CTRL_TRIANGLE, JGE_BTN_CANCEL },
    { PSP_CTRL_SQUARE,   JGE_BTN_PRI },
    { PSP_CTRL_CROSS,    JGE_BTN_SEC },
    { PSP_CTRL_LTRIGGER, JGE_BTN_PREV },
    { PSP_CTRL_RTRIGGER, JGE_BTN_NEXT }
};

void JGECreateDefaultBindings()
{
    for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
        g_engine->BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);
}

int JGEGetTime()
{
    u64 curr;
    sceRtcGetCurrentTick(&curr);
    return (int)((curr * 1000) / gTickFrequency);
}

static SceCtrlData gCtrlPad;

u8 JGEGetAnalogX() { return gCtrlPad.Lx; }
u8 JGEGetAnalogY() { return gCtrlPad.Ly; }

void Run()
{
    static const int keyCodeList[] =
    {
        PSP_CTRL_SELECT, // Select button.
        PSP_CTRL_START, // Start button.
        PSP_CTRL_UP, // Up D-Pad button.
        PSP_CTRL_RIGHT, // Right D-Pad button.
        PSP_CTRL_DOWN, // Down D-Pad button.
        PSP_CTRL_LEFT, // Left D-Pad button.
        PSP_CTRL_LTRIGGER, // Left trigger.
        PSP_CTRL_RTRIGGER, // Right trigger.
        PSP_CTRL_TRIANGLE, // Triangle button.
        PSP_CTRL_CIRCLE, // Circle button.
        PSP_CTRL_CROSS, // Cross button.
        PSP_CTRL_SQUARE, // Square button.
        PSP_CTRL_HOLD, // Hold button.
        /* Do not test keys we cannot get anyway, that's just wasted proc time
        PSP_CTRL_HOME, // Home button.
        PSP_CTRL_NOTE, // Music Note button.
        PSP_CTRL_SCREEN, // Screen button.
        PSP_CTRL_VOLUP, // Volume up button.
        PSP_CTRL_VOLDOWN, // Volume down button.
        PSP_CTRL_WLAN, // _UP    Wlan switch up.
        PSP_CTRL_REMOTE, // Remote hold position.
        PSP_CTRL_DISC, // Disc present.
        PSP_CTRL_MS // Memory stick present.
        */
    };

    u64 curr;
    long long int nextInput = 0;
    u64 lastTime;
    u32 oldButtons;
    u32 veryOldButtons;

    sceRtcGetCurrentTick(&lastTime);
    oldButtons = veryOldButtons = 0;
    JGECreateDefaultBindings();

    while (!g_engine->mDone)
    {
        sceRtcGetCurrentTick(&curr);
        float dt = (curr - lastTime) / (float)gTickFrequency;
        g_engine->mDeltaTime = dt;
        if (!g_engine->mPaused)
        {
            sceCtrlPeekBufferPositive(&gCtrlPad, 1);
            for (signed int i = sizeof(keyCodeList)/sizeof(keyCodeList[0]) - 1; i >= 0; --i)
                if (keyCodeList[i] & gCtrlPad.Buttons)
                {
                    if (!(keyCodeList[i] & oldButtons))
                        g_engine->HoldKey(keyCodeList[i]);
                }
                else
                    if (keyCodeList[i] & oldButtons)
                        g_engine->ReleaseKey(keyCodeList[i]);

            oldButtons = gCtrlPad.Buttons;
            g_engine->Update(dt);
            g_engine->Render();

            if (g_engine->mDebug && strlen(g_engine->mDebuggingMsg) > 0)
            {
                pspDebugScreenSetXY(0, 0);
                pspDebugScreenPrintf(g_engine->mDebuggingMsg);
            }
            veryOldButtons = gCtrlPad.Buttons;
        }
        else
            sceKernelDelayThread(1);
        lastTime = curr;
    }
}

//------------------------------------------------------------------------------------------------
// The main loop
int main(int argc, char *argv[])
{
    JLOG("SetupCallbacks()");
    SetupCallbacks();
#ifdef DEVHOOK
    JLOG("initExceptionHandler()");
    initExceptionHandler();
#endif
    g_engine = NULL;

    JGameLauncher* launcher = new JGameLauncher();

    u32 flags = launcher->GetInitFlags();
    if ((flags&JINIT_FLAG_ENABLE3D) != 0)
        JRenderer::Set3DFlag(true);

    JLOG("sceRtcGetTickResolution()");
    gTickFrequency = sceRtcGetTickResolution();
    JLOG("JGE::GetInstance()");
    g_engine = JGE::GetInstance();
    g_engine->SetARGV(argc, argv);

    JLOG("Create Game");
    game = launcher->GetGameApp();
    game->Create();

    JLOG("Run Game");
    g_engine->SetApp(game);
    g_engine->Run();

    game->Destroy();
    delete game;
    game = NULL;

    g_engine->SetApp(NULL);

    done = true;

    delete launcher;

    JGE::Destroy();
    g_engine = NULL;

    sceKernelExitGame();

    return 0;
}

