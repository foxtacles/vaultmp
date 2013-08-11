#include "vaultscript.h"

namespace vaultmp
{
	namespace Pickup
	{
		VAULTFUNCTION State Register(Function<ID, ID> function, const String& name) noexcept {
			MakePublic(function, name);
			return static_cast<State>(CallPublic("Pickup::Register", name.c_str()));
		}

		VAULTFUNCTION State Register(Function<ID, ID> function, cRawString name) noexcept {
			MakePublic(function, name);
			return static_cast<State>(CallPublic("Pickup::Register", name));
		}
	}
}
