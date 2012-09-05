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

State VAULTSCRIPT OnClientAuthenticate(cRawString name, cRawString pwd) noexcept
{
	return True;
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason) noexcept
{

}

Base VAULTSCRIPT OnPlayerRequestGame(ID player) noexcept
{
	return static_cast<Base>(0x00000000);
}

State VAULTSCRIPT OnPlayerChat(ID player, RawString message) noexcept
{
	return True;
}

Void VAULTSCRIPT OnSpawn(ID object) noexcept
{
	Player player(object);

	if (player)
		player.UIMessage("Hello, " + player.GetName() + "!");
}

Void VAULTSCRIPT OnCellChange(ID object, Cell cell) noexcept
{

}

Void VAULTSCRIPT OnContainerItemChange(ID container, Base item, Count count, Value value) noexcept
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
	AddItem(actor, static_cast<Base>(0x8f216));
EquipItem(actor, static_cast<Base>(0x8f216));
}

Void VAULTSCRIPT OnActorDeath(ID actor, Limb limbs, Death cause) noexcept
{

}

Void VAULTSCRIPT OnActorEquipItem(ID actor, Base item, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorUnequipItem(ID actor, Base item, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorDropItem(ID actor, Base item, UCount count, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorPickupItem(ID actor, Base item, UCount count, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorPunch(ID actor, State power) noexcept
{

}

Void VAULTSCRIPT OnActorFireWeapon(ID actor, Base weapon) noexcept
{

}
