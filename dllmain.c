#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "patch.h"
#include "hook.h"


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



        hook_patch_iat(game_exe, FALSE, "ddraw.dll", "DirectDrawCreate", (PROC)DirectDrawCreate);





        /* Apply patch for original Total Annihilation (OTA) */

        char *text_base = (char*)GetModuleHandleA(NULL) + 0x0C00; /* start=00000400 (mem=00401000) */

        // Make AI commander orders NOT reset whenever attacked.
        // (00406FB4)
        patch_setbyte(text_base + 0x000063B4, 0xEB);
        patch_setbyte(text_base + 0x000063B5, 0x40);

        // ALSO DONE VIA TDRAW
        // Change pathfinding "+search #" to higher initial number (thanks Ti_ and xpoy)
        // http://www.tauniverse.com/forum/showthread.php?t=41608&page=4
        // (0040EAC9)
        patch_setbyte(text_base + 0x0000DED6, 0x5A);
        patch_setbyte(text_base + 0x0000DED7, 0x04);
        patch_setbyte(text_base + 0x0000DED8, 0x01);

        //DirectX popup elimination
        patch_setbyte(text_base + 0x00025AA5, 0xB0);
        patch_setbyte(text_base + 0x00025AA6, 0x01);

        // Enable cursor to be "reclaim" when reclaim cursor hovers over any unit
        // Now Commanders can be anonymous on the minimap
        patch_setbyte(text_base + 0x0003DB86, 0x00);
        patch_setbyte(text_base + 0x0003DB87, 0x00);

        // Disable "\" key from being repeat-last-typed-command
        // (such that "\" will be dedicated demo recorder whiteboard key)
        // (00496694)
        patch_setbyte(text_base + 0x00095AE7, 0x27);

        // Enable debug mode via F10 key. Must be in developers mode to activate.
        patch_setbyte(text_base + 0x00095B76, 0x24);
        patch_setbyte(text_base + 0x00095B79, 0x0D);


        char* rdata_base = (char*)GetModuleHandleA(NULL) + 0x1200; /* start=000FAE00 (mem=004FC000) */
      
        // Increase "+atm" to fill resources (thanks N72)
        // http://www.tauniverse.com/forum/showthread.php?t=41608&page=6
        // (004FCC7C via 004170C0 function)
        patch_setbyte(rdata_base + 0x000FBA7F, 0xD4 );


        char* data_base = (char*)GetModuleHandleA(NULL) + 0x1A00; /* start=000FF600 (mem=00501000) */
        
        // UNKNOWN
        patch_setbyte(data_base + 0x001005D8, 0x02 );
        patch_setbyte(data_base + 0x001005E4, 0x02 );
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
