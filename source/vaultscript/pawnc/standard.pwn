#include <vaultmp>
#include <default/pickup>

forward OnItemPickup(ID, actor);

main()
{

}

public OnServerInit()
{
	printf("My first PAWN vaultscript <3\n");
	SetServerName("vaultmp 0.1a server");
	SetServerRule("website", "vaultmp.com");
	SetServerMap("the wasteland");

	Pickup_Register("OnItemPickup", "vaultscript::OnItemPickup");
}

public OnServerExit(Bool:error)
{

}

public OnGameTimeChange(year, month, day, hour)
{

}

public OnClientAuthenticate(const name{}, const pwd{})
{
	return true;
}

public OnPlayerDisconnect(ID, reason)
{

}

public OnPlayerRequestGame(ID)
{
	return 0x00000000;
}

public OnPlayerChat(ID, message{})
{
	return 1;
}

public OnWindowMode(ID, Bool:enabled)
{

}

public OnWindowClick(ID, player)
{

}

public OnWindowReturn(ID, player)
{

}

public OnWindowTextChange(ID, player, const text{})
{

}

public OnCheckboxSelect(ID, player, Bool:selected)
{

}

public OnRadioButtonSelect(ID, previous, player)
{

}

public OnListItemSelect(ID, player, Bool:selected)
{

}

public OnCreate(ID)
{

}

public OnDestroy(ID)
{

}

public OnSpawn(ID)
{
	if (IsPlayer(ID))
	{
		new message {MAX_MESSAGE_LENGTH};
		GetBaseName(ID, message);

		strformat(message, sizeof(message), true, "Hello, %s!", message);
		UIMessage(ID, message);
	}
}

public OnActivate(ID, actor)
{

}

public OnCellChange(ID, cell)
{

}

public OnLockChange(ID, actor, Lock:lock)
{

}

public OnItemCountChange(ID, count)
{

}

public OnItemConditionChange(ID, Float:condition)
{

}

public OnItemEquippedChange(ID, Bool:equipped)
{

}

public OnItemPickup(ID, actor)
{
	return 1;
}

public OnActorValueChange(ID, ActorValue:index, Float:value)
{

}

public OnActorBaseValueChange(ID, ActorValue:index, Float:value)
{

}

public OnActorAlert(ID, Bool:alerted)
{

}

public OnActorSneak(ID, Bool:sneaking)
{

}

public OnActorDeath(ID, killer, Limb:limbs, Death:cause)
{

}

public OnActorPunch(ID, Bool:power)
{

}

public OnActorFireWeapon(ID, weapon)
{

}