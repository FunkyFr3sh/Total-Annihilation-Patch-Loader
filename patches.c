#include <windows.h>
#include <stdio.h>
#include "ini.h"
#include "patch.h"
#include "patches.h"

#define LOG_ERROR(a, ...) sprintf_s(g_patches_debug, sizeof(g_patches_debug), a, ##__VA_ARGS__);

char g_patches_debug[512];

static int patches_read_ini(void* user, const char* section, const char* name, const char* value)
{
    if (g_patches_debug[0])
    {
        return 1; /* We already had an error, so we're not going to continue here */
    }

#if defined(_DEBUG)
    char debug[256];
    sprintf_s(debug, 256, "[%s]%s=%s\n", section, name, value);
    OutputDebugStringA(debug);
#endif

    if (_strcmpi(name, "patch") == 0)
    {
        const unsigned char* pos = value;
        unsigned char buf[512];
        size_t size = 0;

        while (*pos)
        {
            if (size >= sizeof(buf))
            {
                LOG_ERROR("Patch too long (Limit=512 bytes).\nSection = %s.", section);
                return 0;
            }

            if (!isxdigit(*pos))
            {
                pos++;
                continue;
            }

            if (!isxdigit(pos[1]))
            {
                LOG_ERROR("Single digit hex is not supported.\nErroneous digit = '%c'.", *pos);
                return 0;
            }

            sscanf_s(pos, "%2hhx", &buf[size++]);
            pos += 2;
        }

        DWORD offset = (DWORD)strtol(section, NULL, 16);

        if (offset >= 0x00501000 + 0x0002B000) /* FAIL: reached end of .data section (memory address) */
        {
            LOG_ERROR("Invalid patch offset, value too high.\nSection = %s.", section);
            return 0;
        }
        else if (offset >= 0x00401000) /* .text section start (memory address) */
        {

        }
        else if (offset >= 0x000FF600 + 0x10A00) /* FAIL: reached end of .data section */
        {
            LOG_ERROR("Invalid patch offset, value too high.\nSection = %s.", section);
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
            LOG_ERROR("Invalid patch offset, no valid hex string.\nSection = %s.", section);
            return 0;
        }

        patch_setbytes((char*)offset, buf, size);
    }
    else
    {
        LOG_ERROR("Unknown .ini key name.\nKey = %s.", name);
        return 0;
    }

    return 1;
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
        return FALSE;

    int result = ini_parse_string(ini, patches_read_ini, NULL);
        
    free(ini);

    return result;
}
