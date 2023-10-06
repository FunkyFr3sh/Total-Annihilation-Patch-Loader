#ifndef PATCHES_H 
#define PATCHES_H 

#define GET_MEM_ADDRESS(a) \
    ((char*)GetModuleHandleA(NULL) + (a) + (\
    (a) >= 0x0FF600 + 0x10A00 ? -(int)GetModuleHandleA(NULL) - (a) : \
    (a) >= 0x0FF600 ? 0x1A00 : \
    (a) >= 0x0FAE00 ? 0x1200 : \
    (a) >= 0x000400 ? 0x0C00 : -(int)GetModuleHandleA(NULL) - (a)))

extern char g_patches_debug_str[];

int patches_apply(HMODULE mod);

#endif
