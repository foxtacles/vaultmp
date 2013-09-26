#include "vaultscript.h"

#include <cstdio>

namespace vaultmp
{
	namespace Pickup
	{
		VAULTFUNCTION Result Register(Function<ID, ID> function) noexcept {
			char function_name[16];
			std::snprintf(function_name, sizeof(function_name), "%p", reinterpret_cast<void*>(function));
			MakePublic(function, function_name);
			return CallPublic("Pickup::Register", function_name);
		}
	}
}
