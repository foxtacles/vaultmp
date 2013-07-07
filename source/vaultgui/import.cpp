#include "import.h"

void (*GUI_OnMode)(bool enabled)=0;
void (*GUI_OnChat)(const char* str)=0;

void SetupImports()
{
	GUI_OnMode=(void (*)(bool))GetProcAddress(GetModuleHandle("vaultmp.dll"),"GUI_OnMode");
	GUI_OnChat=(void (*)(const char*))GetProcAddress(GetModuleHandle("vaultmp.dll"),"GUI_OnChat");
}