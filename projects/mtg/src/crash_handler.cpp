// VEH crash handler with symbol pre-loading
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

static HANDLE g_log = INVALID_HANDLE_VALUE;
static volatile LONG g_in_handler = 0;

static void WriteStr(HANDLE h, const char* s) {
    if (h == INVALID_HANDLE_VALUE || !s) return;
    DWORD written; WriteFile(h, s, (DWORD)lstrlenA(s), &written, NULL);
}
static void WriteHex64(HANDLE h, DWORD64 v, int digits = 8) {
    char buf[20]; buf[0]='0'; buf[1]='x';
    for (int i=0; i<digits; i++) {
        int n = (int)((v >> ((digits-1-i)*4)) & 0xF);
        buf[2+i] = n < 10 ? '0'+n : 'A'+n-10;
    }
    buf[2+digits] = 0;
    WriteStr(h, buf);
}

static LONG WINAPI VehCrashFilter(EXCEPTION_POINTERS* ep) {
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (InterlockedCompareExchange(&g_in_handler, 1, 0) != 0)
        return EXCEPTION_CONTINUE_SEARCH;

    WriteStr(g_log, "\r\n=== EXCEPTION code=");
    WriteHex64(g_log, code);
    WriteStr(g_log, " addr=");
    WriteHex64(g_log, (DWORD64)(DWORD_PTR)ep->ExceptionRecord->ExceptionAddress);

    if (code == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2) {
        WriteStr(g_log, " access=");
        WriteStr(g_log, ep->ExceptionRecord->ExceptionInformation[0] == 1 ? "WRITE" : "READ");
        WriteStr(g_log, " at=");
        WriteHex64(g_log, ep->ExceptionRecord->ExceptionInformation[1]);
    }
    WriteStr(g_log, "\r\n");

    HANDLE proc = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    CONTEXT ctx = *ep->ContextRecord;
    STACKFRAME64 sf; memset(&sf, 0, sizeof(sf));
    sf.AddrPC.Offset = ctx.Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = ctx.Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrStack.Offset = ctx.Esp;
    sf.AddrStack.Mode = AddrModeFlat;

    char symBuf[sizeof(SYMBOL_INFO)+512];
    SYMBOL_INFO* sym = (SYMBOL_INFO*)symBuf;

    for (int i=0; i<40; i++) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_I386, proc, thread, &sf, &ctx,
            NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) break;
        if (!sf.AddrPC.Offset) break;

        WriteStr(g_log, "  ");
        WriteHex64(g_log, sf.AddrPC.Offset);
        WriteStr(g_log, " ");

        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 500;
        DWORD64 disp = 0;
        if (SymFromAddr(proc, sf.AddrPC.Offset, &disp, sym)) {
            WriteStr(g_log, sym->Name);
            WriteStr(g_log, "+0x");
            WriteHex64(g_log, disp, 4);
        } else {
            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery((LPCVOID)(DWORD_PTR)sf.AddrPC.Offset, &mbi, sizeof(mbi));
            char mn[MAX_PATH]; mn[0]=0;
            GetModuleFileNameA((HMODULE)mbi.AllocationBase, mn, MAX_PATH);
            WriteStr(g_log, mn[0] ? mn : "(no sym)");
        }
        WriteStr(g_log, "\r\n");
    }

    FlushFileBuffers(g_log);
    InterlockedExchange(&g_in_handler, 0);
    return EXCEPTION_CONTINUE_SEARCH;
}

#pragma init_seg(lib)

struct EarlyCrashInstaller {
    PVOID m_veh;
    EarlyCrashInstaller() : m_veh(NULL) {
        const char* logPath =
            "C:\\Users\\Wolf\\Desktop\\Launcher\\coding\\GitHub\\wagicShowdown\\projects\\mtg\\bin\\crash_trace.txt";
        g_log = CreateFileA(logPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        WriteStr(g_log, "VEH handler installed.\r\n");

        // Pre-load symbols with PDB path
        const char* symPath =
            "C:\\Users\\Wolf\\Desktop\\Launcher\\coding\\GitHub\\wagicShowdown\\projects\\mtg\\Release;"
            "C:\\Users\\Wolf\\Desktop\\Launcher\\coding\\GitHub\\wagicShowdown\\projects\\mtg\\bin;"
            "C:\\Users\\Wolf\\Desktop\\Launcher\\coding\\GitHub\\wagicShowdown\\JGE\\lib\\Release";
        SymInitialize(GetCurrentProcess(), symPath, TRUE);
        WriteStr(g_log, "Symbols loaded.\r\n");
        FlushFileBuffers(g_log);

        m_veh = AddVectoredExceptionHandler(1, VehCrashFilter);
    }
    ~EarlyCrashInstaller() {
        if (m_veh) RemoveVectoredExceptionHandler(m_veh);
        if (g_log != INVALID_HANDLE_VALUE) CloseHandle(g_log);
    }
} g_early_crash_installer;
