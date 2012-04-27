#include "vaultscript.h"
#include <cstdio>

using namespace std;
using namespace vaultmp;

Void VAULTSCRIPT exec()
{
	printf("My first C++ vaultscript <3\n");
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

State VAULTSCRIPT OnClientAuthenticate(string name, string pwd)
{
	return True;
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason)
{

}

Base VAULTSCRIPT OnPlayerRequestGame(ID player)
{
	Base base = (Base) 0x00000000;

	switch (GetGameCode())
	{
		case Index::FALLOUT3:
			base = (Base) 0x00030D82; // Carter
			break;

		case Index::NEWVEGAS:
			base = (Base) 0x0010C0BE; // Jessup
			break;

		default:
			break;
	}

	return base;
}

Void VAULTSCRIPT OnSpawn(ID object)
{
	if (IsPlayer(object))
	{
		UIMessage(object, String("Hello, ") + GetName(object) + "!");

		Base pipboy = (Base) 0x00015038;

		if (GetContainerItemCount(object, pipboy) == 0)
		{
			AddItem(object, pipboy, 1, 100.0, True);
			EquipItem(object, pipboy, True, True);
		}
	}
}

Void VAULTSCRIPT OnCellChange(ID object, Cell cell)
{

}

Void VAULTSCRIPT OnContainerItemChange(ID container, Base base, Count count, Value value)
{

}

Void VAULTSCRIPT OnActorValueChange(ID actor, Index index, Value value)
{

}

Void VAULTSCRIPT OnActorBaseValueChange(ID actor, Index index, Value value)
{

}

Void VAULTSCRIPT OnActorAlert(ID actor, State alerted)
{

}

Void VAULTSCRIPT OnActorSneak(ID actor, State sneaking)
{

}

Void VAULTSCRIPT OnActorDeath(ID actor)
{

}

Void VAULTSCRIPT OnActorEquipItem(ID actor, Base base, Value value)
{

}

Void VAULTSCRIPT OnActorUnequipItem(ID actor, Base base, Value value)
{

}
