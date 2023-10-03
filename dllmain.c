#include <windows.h>
#include <ddraw.h>
#include "patch.h"
#include "patches.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        HMODULE game_exe = GetModuleHandleA(NULL);
        
        if (game_exe && memcmp((char*)game_exe + 0x00010000, "\x14\x68\x78\x1B\x50\x00\x8D\x4C\x24\x1B", 10) != 0)
        {
            MessageBoxA(
                NULL, 
                "Game version not supported. Please install the official 3.1 patch and then try again.", 
                "Total Annihilation Community Patch", 
                MB_OK);

            exit(1);
            return FALSE;
        }

        patches_apply();

        /* Actaully you're not allowed to call LoadLibray from DllMain, but the other patches do it and so we must too */
        HMODULE tdraw_dll = LoadLibraryA("tdraw.dll");

        if (!tdraw_dll)
        {
            MessageBoxA(
                NULL,
                "tdraw.dll not found, please re-install the Total Annihilation Community Patch.",
                "Total Annihilation Community Patch",
                MB_OK);

            exit(1);
            return FALSE;
        }

        FARPROC dd_create = GetProcAddress(tdraw_dll, "DirectDrawCreate");

        if (!dd_create)
        {
            MessageBoxA(
                NULL,
                "Incompatible tdraw.dll found, please re-install the Total Annihilation Community Patch.",
                "Total Annihilation Community Patch",
                MB_OK);

            exit(1);
            return FALSE;
        }

        patch_call((void*)0x0047BFA2, (void*)dd_create);
        patch_call((void*)0x004B55FB, (void*)dd_create);
    }
    
    return TRUE;
}

#pragma comment(linker, "/export:DirectPlayCreate=tplayx.DirectPlayCreate,@1")
#pragma comment(linker, "/export:DirectPlayEnumerateA=tplayx.DirectPlayEnumerateA,@2")
#pragma comment(linker, "/export:DirectPlayEnumerateW=tplayx.DirectPlayEnumerateW,@3")
#pragma comment(linker, "/export:DirectPlayLobbyCreateA=tplayx.DirectPlayLobbyCreateA,@4")
#pragma comment(linker, "/export:DirectPlayLobbyCreateW=tplayx.DirectPlayLobbyCreateW,@5")
#pragma comment(linker, "/export:gdwDPlaySPRefCount=tplayx.gdwDPlaySPRefCount,@6")
#pragma comment(linker, "/export:DirectPlayEnumerate=tplayx.DirectPlayEnumerate,@9")
#pragma comment(linker, "/export:DllCanUnloadNow=tplayx.DllCanUnloadNow,PRIVATE")
#pragma comment(linker, "/export:DllGetClassObject=tplayx.DllGetClassObject,PRIVATE")
