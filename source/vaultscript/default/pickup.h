#include "vaultscript.h"

namespace vaultmp
{
	namespace Pickup
	{
		VAULTFUNCTION Result Register(Function<ID, ID> function, const String& name) noexcept {
			MakePublic(function, name);
			return CallPublic("Pickup::Register", name.c_str());
		}

		VAULTFUNCTION Result Register(Function<ID, ID> function, cRawString name) noexcept {
			MakePublic(function, name);
			return CallPublic("Pickup::Register", name);
		}
	}
}
