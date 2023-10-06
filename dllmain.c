#include <windows.h>
#include <stdio.h>
#include <ddraw.h>
#include "patch.h"
#include "patches.h"

#define TA_DDRAW_DLL_STR     ((char*)0x004FF618)
#define TA_MP_VERSION_MAJOR *((BYTE*)0x0049E9C0)
#define TA_MP_VERSION_MINOR *((BYTE*)0x0049E9C9)

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        HMODULE game_exe = GetModuleHandleA(NULL);
        
        if (!game_exe || memcmp((char*)game_exe + 0x00010000, "\x14\x68\x78\x1B\x50\x00\x8D\x4C\x24\x1B", 10) != 0)
        {
            MessageBoxA(
                NULL, 
                "Game version not supported. Please install the official 3.1 patch and then try again.", 
                "Total Annihilation Community Patch", 
                MB_OK);

            exit(1);
            return FALSE;
        }

        if (strcmp(TA_DDRAW_DLL_STR, "DDRAW.dll") != 0 && (TA_MP_VERSION_MAJOR != 3 || TA_MP_VERSION_MINOR != 1))
        {
            MessageBoxA(
                NULL,
                "Incompatible game files detected. You cannot mix different versions of the Community Patch.\n"
                    "Please perform a clean re-install of Total Annihilation 3.1 and then apply "
                    "the latest Community Patch again.",
                "Total Annihilation Community Patch",
                MB_OK);

            exit(1);
            return FALSE;
        }

        int line = patches_apply(hinstDLL);
        if (line != 0)
        {
            char msg[512] = { 0 };
            sprintf_s(
                msg, 
                sizeof(msg), 
                "Failed to apply game patches. Error between lines %d-%d.\n%s", 
                line - 1, 
                line,
                g_patches_debug_str);
            
            MessageBoxA(NULL, msg, "Total Annihilation Community Patch", MB_OK);

            exit(1);
            return FALSE;
        }

        //return FALSE;
        /* Use byte in code cave as new variable to let tdraw.dll know that the dplayx.dll proxy is active */
        patch_setbyte((void*)0x00401064, 1);

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
