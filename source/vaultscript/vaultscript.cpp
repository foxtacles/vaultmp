#include "vaultscript.h"
#include <cstdio>

using namespace vaultmp;

Void VAULTSCRIPT exec() noexcept
{
	std::printf("My first C++ vaultscript <3\n");
	SetServerName("vaultmp 0.1a server");
	SetServerRule("website", "vaultmp.com");

	switch (GetGameCode())
	{
		case Index::FALLOUT3:
			SetServerMap("the wasteland");
			break;

		case Index::NEWVEGAS:
			SetServerMap("mojave desert");
			break;

		default:
			break;
	}
}

State VAULTSCRIPT OnClientAuthenticate(String name, String pwd) noexcept
{
	return True;
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason) noexcept
{

}

Base VAULTSCRIPT OnPlayerRequestGame(ID player) noexcept
{
	Base base = static_cast<Base>(0);

	switch (GetGameCode())
	{
		case Index::FALLOUT3:
			base = static_cast<Base>(0x00030D82); // Carter
			break;

		case Index::NEWVEGAS:
			base = static_cast<Base>(0x0010C0BE); // Jessup
			break;

		default:
			break;
	}

	return base;
}

State VAULTSCRIPT OnPlayerChat(ID player, RawString message) noexcept
{
	return True;
}

Void VAULTSCRIPT OnSpawn(ID object) noexcept
{
	Player player(object);

	if (player)
	{
		player.UIMessage("Hello, " + player.GetName() + "!");

		Base pipboy = static_cast<Base>(0x00015038);

		if (player.GetContainerItemCount(pipboy) == 0)
		{
			player.AddItem(pipboy);
			player.EquipItem(pipboy);
		}
	}
}

Void VAULTSCRIPT OnCellChange(ID object, Cell cell) noexcept
{

}

Void VAULTSCRIPT OnContainerItemChange(ID container, Base base, Count count, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorValueChange(ID actor, Index index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorBaseValueChange(ID actor, Index index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorAlert(ID actor, State alerted) noexcept
{

}

Void VAULTSCRIPT OnActorSneak(ID actor, State sneaking) noexcept
{

}

Void VAULTSCRIPT OnActorDeath(ID actor) noexcept
{

}

Void VAULTSCRIPT OnActorEquipItem(ID actor, Base base, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorUnequipItem(ID actor, Base base, Value value) noexcept
{

}

