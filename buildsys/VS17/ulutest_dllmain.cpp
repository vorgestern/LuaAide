
#include <Windows.h>
#include <cstdio>
#include <io.h>

HMODULE ulutest_module=nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            ulutest_module=hModule;
            // printf("DllMain PROCESS_ATTACH: (%p, %u, %p)\n", hModule, ul_reason_for_call, lpReserved);
            break;
        case DLL_THREAD_ATTACH:
            // printf("DllMain THREAD_ATTACH: (%p, %u, %p)\n", hModule, ul_reason_for_call, lpReserved);
            break;
        case DLL_THREAD_DETACH:
            // printf("DllMain THREAD_DETACH: (%p, %u, %p)\n", hModule, ul_reason_for_call, lpReserved);
            break;
        case DLL_PROCESS_DETACH:
            // printf("DllMain PROCESS_DETACH: (%p, %u, %p)\n", hModule, ul_reason_for_call, lpReserved);
            break;
    }
    return TRUE;
}

bool check_tty(int id) { return _isatty(id); }
