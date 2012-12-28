#include <vaultmp>

main()
{
	printf("My first PAWN vaultscript <3\n");
	SetServerName("vaultmp 0.1a server");
	SetServerRule("website", "vaultmp.com");

	switch (GetGameCode())
	{
		case FALLOUT3:
			SetServerMap("the wasteland");

		case NEWVEGAS:
			SetServerMap("mojave desert");
	}
}

public OnServerInit()
{

}

public OnServerExit()
{

}

public OnGameYearChange(year)
{

}

public OnGameMonthChange(month)
{

}

public OnGameDayChange(day)
{

}

public OnGameHourChange(hour)
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

public OnSpawn(ID)
{
	if (IsPlayer(ID))
	{
		new message {MAX_MESSAGE_LENGTH};
		GetName(ID, message);

		strformat(message, sizeof(message), true, "Hello, %s!", message);
		UIMessage(ID, message);
	}
}

public OnCellChange(ID, cell)
{

}

public OnContainerItemChange(ID, item, count, Float: value)
{

}

public OnActorValueChange(ID, index, Float: value)
{

}

public OnActorBaseValueChange(ID, index, Float: value)
{

}

public OnActorAlert(ID, Bool: alerted)
{

}

public OnActorSneak(ID, Bool: sneaking)
{

}

public OnActorDeath(ID, Limb: limbs, Death: cause)
{

}

public OnActorEquipItem(ID, item, Float: value)
{

}

public OnActorUnequipItem(ID, item, Float: value)
{

}

public OnActorDropItem(ID, item, count, Float: value)
{

}

public OnActorPickupItem(ID, item, count, Float: value)
{

}

public OnActorPunch(ID, Bool:power)
{

}

public OnActorFireWeapon(ID, weapon)
{

}