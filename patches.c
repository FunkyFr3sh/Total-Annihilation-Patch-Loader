#include <windows.h>
#include <stdio.h>
#include "ini.h"
#include "patch.h"
#include "patches.h"

#define LOG_ERROR(a, ...) sprintf_s(g_patches_debug_str, sizeof(g_patches_debug_str), a, ##__VA_ARGS__);

char g_patches_debug_str[512];
static PATCH_OFFSET g_patches_offsets[4096];

static int patches_setbytes(DWORD offset, char* buf, size_t size)
{
    if (offset == 0)
    {
        LOG_ERROR("Invalid patch offset.\nOffset = %08X", offset);
        return 0;
    }

    if (size == 0)
    {
        LOG_ERROR("Empty patch (0 bytes) detected.\nOffset = %08X (%08X)", GET_PHYS_OFFSET(offset), offset);
        return 0;
    }

    for (int i = 0; i < sizeof(g_patches_offsets) / sizeof(g_patches_offsets[0]); i++)
    {
        if (g_patches_offsets[i].start)
        {
            if ((offset            >= g_patches_offsets[i].start && offset            <  g_patches_offsets[i].end) ||
                (offset + size - 1 >= g_patches_offsets[i].start && offset + size - 1 <  g_patches_offsets[i].end) ||
                (offset            <  g_patches_offsets[i].start && offset + size - 1 >= g_patches_offsets[i].start))
            {
                LOG_ERROR(
                    "Patch '%08X (%08X)' is conflicting with:\nPatch '%08X (%08X)'", 
                    GET_PHYS_OFFSET(offset),
                    offset,
                    GET_PHYS_OFFSET(g_patches_offsets[i].start),
                    g_patches_offsets[i].start);

                return 0;
            }

            continue;
        }
        else
        {
            g_patches_offsets[i].start = offset;
            g_patches_offsets[i].end = offset + size;

            patch_setbytes((void*)offset, buf, size);
            return 1;
        }
    }

    LOG_ERROR("Too many patches, limit reached.\nLimit = %d", sizeof(g_patches_offsets) / sizeof(g_patches_offsets[0]));
    return 0;
}

static int patches_setbyte(DWORD offset, BYTE value)
{
    return patches_setbytes(offset, (char*)&value, 1);
}

static int patches_apply_presets(void* user, const char* section, const char* name, const char* value)
{
    if (!name || !value)
    {
        return 1; /* start of a new section (Enabled to get proper line numbers on debug logs) */
    }

    if (_strcmpi(name, "RegistryPath") == 0)
    {
        if (strlen(value) >= 1 && strlen(value) <= 21)
        {
            // Use custom registry path
            // Replaces "Software\Cavedog Entertainment" with "Software\..."

            if (!patches_setbytes(0x0050DDFD, (char*)value, strlen(value) + 1))
                return 0;

            if (!patches_setbytes(0x00509EB8, (char*)value, strlen(value) + 1))
                return 0;
        }
        else
        {
            LOG_ERROR("Invalid value length\n[%s]%s=%s\nValid length 1-21", section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "ConfigFileName") == 0)
    {
        if (strlen(value) >= 1 && strlen(value) <= 12)
        {
            // Use custom config file name

            if (!patches_setbytes(0x005098A3, (char*)value, strlen(value) + 1))
                return 0;
        }
        else
        {
            LOG_ERROR("Invalid value length\n[%s]%s=%s\nValid length 1-12", section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "Gp3FileName") == 0)
    {
        if (strlen(value) >= 1 && strlen(value) <= 11)
        {
            // Use custom .GP3 file name

            if (!patches_setbytes(0x005028CC, (char*)value, strlen(value) + 1))
                return 0;
        }
        else
        {
            LOG_ERROR("Invalid value length\n[%s]%s=%s\nValid length 1-11", section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "DownloadPath") == 0)
    {
        if (strlen(value) >= 1 && strlen(value) <= 11)
        {
            // Use custom download path

            if (!patches_setbytes(0x00503730, (char*)value, strlen(value) + 1))
                return 0;
        }
        else
        {
            LOG_ERROR("Invalid value length\n[%s]%s=%s\nValid length 1-11", section, name, value);
            return 0;
        }
    }
    else if (_strcmpi(name, "GameVersionString") == 0)
    {
        if (strlen(value) <= 7)
        {
            // Change version string (DebugString...)

            if (!patches_setbytes(0x005031B4, (char*)value, strlen(value) + 1))
                return 0;
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

            if (!patches_setbyte(0x0049E9C0, (BYTE)strtol(value, NULL, 10)))
                return 0;
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

            if (!patches_setbyte(0x0049E9C9, (BYTE)strtol(value, NULL, 10)))
                return 0;
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

            if (!patches_setbyte(GET_MEM_ADDRESS(0x000063B4), 0xEB))
                return 0;

            if (!patches_setbyte(GET_MEM_ADDRESS(0x000063B5), 0x40))
                return 0;
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

            if (!patches_setbyte(GET_MEM_ADDRESS(0x0000DED6), 0x5A))
                return 0;

            if (!patches_setbyte(GET_MEM_ADDRESS(0x0000DED7), 0x04))
                return 0;

            if (!patches_setbyte(GET_MEM_ADDRESS(0x0000DED8), 0x01))
                return 0;
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

            if (!patches_setbyte(GET_MEM_ADDRESS(0x00025AA5), 0xB0))
                return 0;

            if (!patches_setbyte(GET_MEM_ADDRESS(0x00025AA6), 0x01))
                return 0;
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

            if (!patches_setbyte(GET_MEM_ADDRESS(0x0003DB86), 0x00))
                return 0;

            if (!patches_setbyte(GET_MEM_ADDRESS(0x0003DB87), 0x00))
                return 0;
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

            if (!patches_setbyte(GET_MEM_ADDRESS(0x00095AE7), 0x27))
                return 0;
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

            if (!patches_setbyte(GET_MEM_ADDRESS(0x00095B76), 0x24))
                return 0;

            if (!patches_setbyte(GET_MEM_ADDRESS(0x00095B79), 0x0D))
                return 0;
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

            if (!patches_setbyte(GET_MEM_ADDRESS(0x000FBA7F), 0xD4))
                return 0;
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

    if (!value)
    {
        return 1; /* start of a new section (Enabled to get proper line numbers on debug logs) */
    }

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

    if (size == 0)
    {
        LOG_ERROR("Empty patch (0 bytes) detected.\nSection = %s", section);
        return 0;
    }

    return patches_setbytes(offset, (char*)buf, size);
}

static int patches_read_ini(void* user, const char* section, const char* name, const char* value)
{
#if defined(_DEBUG)
    char debug[1536];
    sprintf_s(debug, 1536, "[%s]%s=%s\n", section, name, value);
    OutputDebugStringA(debug);
#endif

    if (_strcmpi(section, "Settings") == 0)
    {
        return patches_apply_presets(user, section, name, value);
    }

    if (!name || _strcmpi(name, "patch") == 0)
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
