#ifndef PAWN_H
#define PAWN_H

#include "vaultserver.h"
#include "amx/amx.h"
#include "boost/any.hpp"

/**
 * \brief Contains the PAWN scripting function wrappers
 */

class PAWN
{
	private:
		PAWN() = delete;

		static AMX_NATIVE_INFO functions[];

		static cell CreateTimer(AMX* amx, const cell* params) noexcept;
		static cell CreateTimerEx(AMX* amx, const cell* params) noexcept;
		static cell MakePublic(AMX* amx, const cell* params) noexcept;
		static cell CallPublic(AMX* amx, const cell* params) noexcept;

	public:
		static int LoadProgram(AMX* amx, const char* filename, void* memblock);
		static int Init(AMX* amx);
		static int Exec(AMX* amx, cell* retval, int index);
		static int FreeProgram(AMX* amx);
		static bool IsCallbackPresent(AMX* amx, const char* name);
		static cell Call(AMX* amx, const char* name, const char* argl, int buf, ...);
		static cell Call(AMX* amx, const char* name, const char* argl, const std::vector<boost::any>& args);
};

#endif
