#include <windows.h>
#include <stdio.h>
#include "patch.h"
#include "patches.h"


void patches_apply()
{
    unsigned char sections[2048] = { 0 };

    if (GetPrivateProfileSectionNamesA(sections, sizeof(sections), ".\\patches.ini") == 0)
        return;

    unsigned char patch[1024] = { 0 };
    char* base = (char*)GetModuleHandleA(NULL);

    for (char* s = sections; *s; s += strlen(s) + 1)
    {
        if (GetPrivateProfileStringA(s, "patch", "", patch, sizeof(patch), ".\\patches.ini") >= 2)
        {
            const unsigned char* pos = patch;
            unsigned char buf[512];
            size_t size = 0;
           
            while(*pos) 
            {
                if (!isxdigit(*pos))
                {
                    pos++;
                    continue;
                }

                sscanf_s(pos, "%2hhx", &buf[size++]);
                pos += 2;
            }

            DWORD offset = (DWORD)strtol(s, NULL, 16);
            
            if (offset >= 0x00401000) /* .text section start (memory address) */
            {

            }
            else if (offset >= 0x000FF600) /* .data start=000FF600 (mem=00501000) */
            {
                offset += 0x1A00;
            }
            else if (offset >= 0x000FAE00) /* .rdata start=000FAE00 (mem=004FC000) */
            {
                offset += 0x1200;
            }
            else if (offset >= 0x00000400) /* .text start=00000400 (mem=00401000) */
            {
                offset += 0x0C00;
            }

            patch_setbytes(base + offset, buf, size);
        }
    }
}
