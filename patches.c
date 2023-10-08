#include <windows.h>
#include <stdio.h>
#include "ini.h"
#include "patch.h"
#include "patches.h"

#define LOG_ERROR(a, ...) sprintf_s(g_patches_debug_str, sizeof(g_patches_debug_str), a, ##__VA_ARGS__);

char g_patches_debug_str[512];

static int patches_apply_presets(void* user, const char* section, const char* name, const char* value)
{
    if (_strcmpi(name, "RegistryPath") == 0)
    {
        if (strlen(value) >= 1 && strlen(value) <= 21)
        {
            // Use custom registry path
            // Replaces "Software\Cavedog Entertainment" with "Software\..."
            patch_setbytes((void*)0x0050DDFD, (char*)value, strlen(value) + 1);
            patch_setbytes((void*)0x00509EB8, (char*)value, strlen(value) + 1);
        }
        else
        {
            LOG_ERROR("Invalid value length\n[%s]%s=%s\nValid length 1-21", section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "GameVersionString") == 0)
    {
        if (strlen(value) <= 7)
        {
            // Change version string (DebugString...)
            patch_setbytes((void*)0x005031B4, (char*)value, strlen(value) + 1);
        }
        else
        {
            LOG_ERROR("Value too long - '%s'\n[%s]%s=%s\nValid length 0-7", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "MultiplayerVersionMajor") == 0)
    {
        if (strlen(value) >= 1 && strspn(value, "0123456789") == strlen(value) && strtol(value, NULL, 10) <= 127)
        {
            // Change version # in multiplayer battleroom (all players must match)
            // (0049E9BD)
            patch_setbyte((void*)0x0049E9C0, (BYTE)strtol(value, NULL, 10));
        }
        else
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 0-127", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "MultiplayerVersionMinor") == 0)
    {
        if (strlen(value) >= 1 && strspn(value, "0123456789") == strlen(value) && strtol(value, NULL, 10) <= 127)
        {
            // Change version # in multiplayer battleroom (all players must match)
            // (0049E9BD)
            patch_setbyte((void*)0x0049E9C9, (BYTE)strtol(value, NULL, 10));
        }
        else
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 0-127", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "CommanderOrdersNotReset") == 0)
    {
        if (_strcmpi(value, "Yes") == 0)
        {
            // Make AI commander orders NOT reset whenever attacked.
            // (00406FB4)
            patch_setbyte(GET_MEM_ADDRESS(0x000063B4), 0xEB);
            patch_setbyte(GET_MEM_ADDRESS(0x000063B5), 0x40);
        }
        else if (_strcmpi(value, "No") != 0)
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 'Yes' and 'No'", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "ChangePathfindingSearch") == 0)
    {
        if (_strcmpi(value, "Yes") == 0)
        {
            // ALSO DONE VIA TDRAW
            // Change pathfinding "+search #" to higher initial number (thanks Ti_ and xpoy)
            // http://www.tauniverse.com/forum/showthread.php?t=41608&page=4
            // (0040EAC9)
            patch_setbyte(GET_MEM_ADDRESS(0x0000DED6), 0x5A);
            patch_setbyte(GET_MEM_ADDRESS(0x0000DED7), 0x04);
            patch_setbyte(GET_MEM_ADDRESS(0x0000DED8), 0x01);
        }
        else if (_strcmpi(value, "No") != 0)
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 'Yes' and 'No'", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "DirectxPopupElimination") == 0)
    {
        if (_strcmpi(value, "Yes") == 0)
        {
            // DirectX popup elimination
            patch_setbyte(GET_MEM_ADDRESS(0x00025AA5), 0xB0);
            patch_setbyte(GET_MEM_ADDRESS(0x00025AA6), 0x01);
        }
        else if (_strcmpi(value, "No") != 0)
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 'Yes' and 'No'", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "CursorReclaim") == 0)
    {
        if (_strcmpi(value, "Yes") == 0)
        {
            // Enable cursor to be "reclaim" when reclaim cursor hovers over any unit
            // Now Commanders can be anonymous on the minimap
            patch_setbyte(GET_MEM_ADDRESS(0x0003DB86), 0x00);
            patch_setbyte(GET_MEM_ADDRESS(0x0003DB87), 0x00);
        }
        else if (_strcmpi(value, "No") != 0)
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 'Yes' and 'No'", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "DisableKeyLastCommandRepeat") == 0)
    {
        if (_strcmpi(value, "Yes") == 0)
        {
            // Disable "\" key from being repeat-last-typed-command
            // (such that "\" will be dedicated demo recorder whiteboard key)
            // (00496694)
            patch_setbyte(GET_MEM_ADDRESS(0x00095AE7), 0x27);
        }
        else if (_strcmpi(value, "No") != 0)
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 'Yes' and 'No'", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "EnableF10Debug") == 0)
    {
        if (_strcmpi(value, "Yes") == 0)
        {
            // Enable debug mode via F10 key. Must be in developers mode to activate.
            patch_setbyte(GET_MEM_ADDRESS(0x00095B76), 0x24);
            patch_setbyte(GET_MEM_ADDRESS(0x00095B79), 0x0D);
        }
        else if (_strcmpi(value, "No") != 0)
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 'Yes' and 'No'", value, section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "IncreaseAtmToFillResources") == 0)
    {
        if (_strcmpi(value, "Yes") == 0)
        {
            // Increase "+atm" to fill resources (thanks N72)
            // http://www.tauniverse.com/forum/showthread.php?t=41608&page=6
            // (004FCC7C via 004170C0 function)
            patch_setbyte(GET_MEM_ADDRESS(0x000FBA7F), 0xD4);
        }
        else if (_strcmpi(value, "No") != 0)
        {
            LOG_ERROR("Invalid value - '%s'\n[%s]%s=%s\nValid values = 'Yes' and 'No'", value, section, name, value);
            return 0;
        }
    }
    else
    {
        LOG_ERROR("Unknown setting - '%s'\n[%s]%s=%s", name, section, name, value);
        return 0;
    }

    return 1;
}

static int patches_apply_customs(void* user, const char* section, const char* value)
{
    const unsigned char* pos = (const unsigned char*)value;
    unsigned char buf[512];
    size_t size = 0;

    while (*pos)
    {
        if (size >= sizeof(buf))
        {
            LOG_ERROR("Patch too long.\nSection = %s\nValid length = %d bytes", section, sizeof(buf));
            return 0;
        }

        if (!isxdigit(*pos))
        {
            pos++;
            continue;
        }

        if (!isxdigit(pos[1]))
        {
            LOG_ERROR("Single digit hex is not supported.\nErroneous digit = '%c'\nSection = %s", *pos, section);
            return 0;
        }

        sscanf_s(pos, "%2hhx", &buf[size++]);
        pos += 2;
    }

    DWORD offset = (DWORD)strtol(section, NULL, 16);

    if (offset >= 0x00501000 + 0x0002B000) /* FAIL: reached end of .data section (memory address) */
    {
        LOG_ERROR("Invalid patch offset, value too high.\nSection = %s", section);
        return 0;
    }
    else if (offset >= 0x00401000) /* .text section start (memory address) */
    {

    }
    else if (offset >= 0x000FF600 + 0x10A00) /* FAIL: reached end of .data section */
    {
        LOG_ERROR("Invalid patch offset, value too high.\nSection = %s", section);
        return 0;
    }
    else if (offset >= 0x000FF600) /* .data start=000FF600 (mem=00501000) */
    {
        offset += (DWORD)GetModuleHandleA(NULL) + 0x1A00;
    }
    else if (offset >= 0x000FAE00) /* .rdata start=000FAE00 (mem=004FC000) */
    {
        offset += (DWORD)GetModuleHandleA(NULL) + 0x1200;
    }
    else if (offset >= 0x00000400) /* .text start=00000400 (mem=00401000) */
    {
        offset += (DWORD)GetModuleHandleA(NULL) + 0x0C00;
    }
    else
    {
        LOG_ERROR("Invalid patch offset, no valid hex string.\nSection = %s", section);
        return 0;
    }

    patch_setbytes((char*)offset, (char*)buf, size);

    return 1;
}

static int patches_read_ini(void* user, const char* section, const char* name, const char* value)
{
    if (g_patches_debug_str[0])
    {
        return 1; /* We already had an error, so we're not going to continue here */
    }

#if defined(_DEBUG)
    char debug[256];
    sprintf_s(debug, 256, "[%s]%s=%s\n", section, name, value);
    OutputDebugStringA(debug);
#endif

    if (_strcmpi(section, "Settings") == 0)
    {
        return patches_apply_presets(user, section, name, value);
    }

    if (_strcmpi(name, "patch") == 0)
    {
        return patches_apply_customs(user, section, value);
    }

    LOG_ERROR("Unknown .ini key name - '%s'\n[%s]%s=", name, section, name);
    return 0;
}

int patches_apply(HMODULE mod)
{
    void* ini = NULL;

    HRSRC loc = FindResourceA(mod, MAKEINTRESOURCE(1000), RT_RCDATA);
    if (loc)
    {
        HGLOBAL res = LoadResource(mod, loc);
        DWORD size = SizeofResource(mod, loc);
        if (res && size > 0)
        {
            void *data = LockResource(res);
            if (data)
            {
                ini = calloc(1, size + 10);
                if (ini)
                {
                    memcpy(ini, data, size);
                }
            }
        }
    }

    if (!ini)
    {
        LOG_ERROR("Patch settings file not found.\nPlease check the resources with Resource Hacker.");
        return -1;
    }

    int result = ini_parse_string(ini, patches_read_ini, NULL);
        
    free(ini);

    return result;
}
