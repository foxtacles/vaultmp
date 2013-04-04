#include "vaultscript.h"
#include <cstdio>

using namespace vaultmp;

Void VAULTSCRIPT exec() noexcept
{
	std::printf("My first C++ vaultscript <3\n");
	SetServerName("vaultmp 0.1a server");
	SetServerRule("website", "vaultmp.com");
	SetServerMap("the wasteland");
}

Void VAULTSCRIPT OnServerInit() noexcept
{

}

Void VAULTSCRIPT OnServerExit() noexcept
{

}

Void VAULTSCRIPT OnGameYearChange(UCount year) noexcept
{

}

Void VAULTSCRIPT OnGameMonthChange(UCount month) noexcept
{

}

Void VAULTSCRIPT OnGameDayChange(UCount day) noexcept
{

}

Void VAULTSCRIPT OnGameHourChange(UCount hour) noexcept
{

}

State VAULTSCRIPT OnClientAuthenticate(cRawString name, cRawString pwd) noexcept
{
	return True;
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason) noexcept
{

}

NPC_ VAULTSCRIPT OnPlayerRequestGame(ID player) noexcept
{
	return static_cast<NPC_>(0x00000000);
}

State VAULTSCRIPT OnPlayerChat(ID player, RawString message) noexcept
{
	return True;
}

Void VAULTSCRIPT OnSpawn(ID object) noexcept
{
	Player player(object);

	if (player)
		player.UIMessage("Hello, " + player.GetBaseName() + "!");
}

Void VAULTSCRIPT OnCellChange(ID object, CELL cell) noexcept
{

}

Void VAULTSCRIPT OnLockChange(ID object, ID player, Lock lock) noexcept
{

}

Void VAULTSCRIPT OnContainerItemChange(ID container, Base item, Count count, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorValueChange(ID actor, ActorValue index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorBaseValueChange(ID actor, ActorValue index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorAlert(ID actor, State alerted) noexcept
{

}

Void VAULTSCRIPT OnActorSneak(ID actor, State sneaking) noexcept
{

}

Void VAULTSCRIPT OnActorDeath(ID actor, ID killer, Limb limbs, Death cause) noexcept
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

Void VAULTSCRIPT OnActorPickupItem(ID actor, Base item, UCount count, Value value, NPC_ owner) noexcept
{

}

Void VAULTSCRIPT OnActorPunch(ID actor, State power) noexcept
{

}

Void VAULTSCRIPT OnActorFireWeapon(ID actor, WEAP weapon) noexcept
{

}
