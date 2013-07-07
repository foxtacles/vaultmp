#include <Windows.h>

extern void (*GUI_OnMode)(bool enabled);
extern void (*GUI_OnChat)(const char* str);

void SetupImports();