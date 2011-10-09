#ifndef VAULTDETOUR_H
#define VAULTDETOUR_H

void* DetourFunction( void* from, void* to ); // returns a ptr to a trampoline for the detoured function. minor memory leak, yes.
void RestoreFunction( void* from, void* tramp ); // todo:
void* DetourForSteam( void* from, void* to ); // checks if the *from is already hooked, and if it is, then follows the hook and writes a hook there.
// minor TODO: create a RestoreFunction(void* from, void* trampoline); that reads the trampoline looking for a 0xE9 reentry jmp to determine the size of the overwritten opcodes, and then writes them back to from

#endif
