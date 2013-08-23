#include "vaultscript.h"

namespace vaultmp
{
	namespace Conversation
	{
		VAULTFUNCTION Result Open(ID id, ID with_id) noexcept {
			return CallPublic("Conversation::Open", id, with_id);
		}

		VAULTFUNCTION Result Toggle(ID id, ID with_id, State toggle) noexcept {
			return CallPublic("Conversation::Toggle", id, with_id, toggle);
		}

		VAULTFUNCTION Result Close(ID id, ID with_id) noexcept {
			return CallPublic("Conversation::Close", id, with_id);
		}
	}
}
