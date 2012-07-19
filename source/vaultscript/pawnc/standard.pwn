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

public OnClientAuthenticate(const name {}, const pwd {})
{
	return true;
}

public OnPlayerDisconnect(ID, reason)
{
}

public OnPlayerRequestGame(ID)
{
	new base = 0x00000000;

	switch (GetGameCode())
	{
		case FALLOUT3:
			base = 0x00030D82; // Carter

		case NEWVEGAS:
			base = 0x0010C0BE; // Jessup
	}

	return base;
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

		new pipboy = 0x00015038;

		if (GetContainerItemCount(ID, pipboy) == 0)
		{
			AddItem(ID, pipboy, 1, 100.0, Bool:true);
			EquipItem(ID, pipboy, Bool:true, Bool:true);
		}
	}
}

public OnCellChange(ID, cell)
{

}

public OnContainerItemChange(ID, base, count, Float: value)
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

public OnActorEquipItem(ID, base, Float: value)
{

}

public OnActorUnequipItem(ID, base, Float: value)
{

}
