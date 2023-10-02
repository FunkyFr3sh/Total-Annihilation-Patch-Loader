#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include "hook.h"


void hook_patch_iat(HMODULE hmod, BOOL unhook, char* module_name, char* function_name, PROC new_function)
{
    HOOKLIST hooks[2];
    memset(&hooks, 0, sizeof(hooks));

    hooks[0].data[0].new_function = new_function;

    strncpy_s(
        hooks[0].module_name, 
        sizeof(hooks[0].module_name) - 1,
        module_name, 
        sizeof(hooks[0].module_name) - 1);

    strncpy_s(
        hooks[0].data[0].function_name, 
        sizeof(hooks[0].data[0].function_name) - 1,
        function_name,  
        sizeof(hooks[0].data[0].function_name) - 1);

    hook_patch_iat_list(hmod, unhook, (HOOKLIST*)&hooks);
}

void hook_patch_iat_list(HMODULE hmod, BOOL unhook, HOOKLIST* hooks)
{
    if (!hmod || hmod == INVALID_HANDLE_VALUE || !hooks)
        return;

    __try
    {
        PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hmod;
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            return;

        PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((DWORD)dos_header + (DWORD)dos_header->e_lfanew);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            return;

        PIMAGE_IMPORT_DESCRIPTOR import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)dos_header +
            (DWORD)(nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));

        if (import_desc == (PIMAGE_IMPORT_DESCRIPTOR)nt_headers)
            return;

        while (import_desc->FirstThunk)
        {
            for (int i = 0; hooks[i].module_name[0]; i++)
            {
                char* imp_module_name = (char*)((DWORD)dos_header + (DWORD)(import_desc->Name));

                if (_stricmp(imp_module_name, hooks[i].module_name) == 0)
                {
                    PIMAGE_THUNK_DATA first_thunk =
                        (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->FirstThunk);

                    PIMAGE_THUNK_DATA original_first_thunk =
                        (PIMAGE_THUNK_DATA)((DWORD)dos_header + (DWORD)import_desc->OriginalFirstThunk);

                    while (first_thunk->u1.Function && original_first_thunk->u1.AddressOfData)
                    {
                        PIMAGE_IMPORT_BY_NAME import =
                            (PIMAGE_IMPORT_BY_NAME)((DWORD)dos_header + original_first_thunk->u1.AddressOfData);

                        if ((original_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)
                        {
                            for (int x = 0; hooks[i].data[x].function_name[0]; x++)
                            {
                                if (!unhook && !hooks[i].data[x].new_function)
                                    continue;

                                if (_stricmp((const char*)import->Name, hooks[i].data[x].function_name) == 0)
                                {
                                    DWORD op;

                                    if (VirtualProtect(
                                        &first_thunk->u1.Function, 
                                        sizeof(DWORD), 
                                        PAGE_READWRITE, 
                                        &op))
                                    {
                                        if (unhook)
                                        {
                                            DWORD org =
                                                (DWORD)GetProcAddress(
                                                    GetModuleHandle(hooks[i].module_name),
                                                    hooks[i].data[x].function_name);

                                            if (org && first_thunk->u1.Function != org)
                                            {
                                                first_thunk->u1.Function = org;
                                            }
                                        }
                                        else
                                        {
                                            if (first_thunk->u1.Function != (DWORD)hooks[i].data[x].new_function)
                                                first_thunk->u1.Function = (DWORD)hooks[i].data[x].new_function;
                                        }

                                        VirtualProtect(&first_thunk->u1.Function, sizeof(DWORD), op, &op);
                                    }

                                    break;
                                }
                            }
                        }

                        first_thunk++;
                        original_first_thunk++;
                    }
                }
            }

            import_desc++;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
