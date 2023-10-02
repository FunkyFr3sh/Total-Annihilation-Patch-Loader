#ifndef HOOK_H 
#define HOOK_H 

typedef struct HOOKLISTDATA { char function_name[32]; PROC new_function; PROC* function; DWORD flags; } HOOKLISTDATA;
typedef struct HOOKLIST { char module_name[32]; HOOKLISTDATA data[30]; } HOOKLIST;

void hook_patch_iat(HMODULE hmod, BOOL unhook, char* module_name, char* function_name, PROC new_function);
void hook_patch_iat_list(HMODULE hmod, BOOL unhook, HOOKLIST* hooks);

#endif
