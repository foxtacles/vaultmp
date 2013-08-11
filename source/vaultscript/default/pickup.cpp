#include "vaultscript.h"

#include <algorithm>

using namespace vaultmp;
using namespace std;

vector<string> callbacks;
Result VAULTSCRIPT RegisterPickup(cRawString name) noexcept;

Void VAULTSCRIPT OnServerInit() noexcept
{
	MakePublic(RegisterPickup, "Pickup::Register");
}

Result VAULTSCRIPT RegisterPickup(cRawString name) noexcept
{
	callbacks.emplace_back(name);
	return static_cast<Result>(True);
}

Void VAULTSCRIPT OnActivate(ID object, ID actor) noexcept
{
	Item item(object);

	if (item)
	{
		State result = True;
		for_each(callbacks.begin(), callbacks.end(), [&result, object, actor](const String& name) { result = static_cast<State>(CallPublic(name.c_str(), object, actor)); });

		if (result)
			item.SetItemContainer(actor);
	}
}
