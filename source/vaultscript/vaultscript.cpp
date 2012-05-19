#include "vaultscript.h"
#include <cstdio>

using namespace vaultmp;

Void VAULTSCRIPT exec() _CPP(noexcept)
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

State VAULTSCRIPT OnClientAuthenticate(String name, String pwd) _CPP(noexcept)
{
	return True;
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason) _CPP(noexcept)
{

}

Base VAULTSCRIPT OnPlayerRequestGame(ID player) _CPP(noexcept)
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

Void VAULTSCRIPT OnSpawn(ID object) _CPP(noexcept)
{
	if (IsPlayer(object))
	{
		UIMessage(object, String("Hello, ") + GetName(object) + "!");

		Base pipboy = (Base) 0x00015038;

		if (GetContainerItemCount(object, pipboy) == 0)
		{
			AddItem(object, pipboy);
			EquipItem(object, pipboy);
		}
	}
}

Void VAULTSCRIPT OnCellChange(ID object, Cell cell) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnContainerItemChange(ID container, Base base, Count count, Value value) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnActorValueChange(ID actor, Index index, Value value) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnActorBaseValueChange(ID actor, Index index, Value value) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnActorAlert(ID actor, State alerted) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnActorSneak(ID actor, State sneaking) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnActorDeath(ID actor) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnActorEquipItem(ID actor, Base base, Value value) _CPP(noexcept)
{

}

Void VAULTSCRIPT OnActorUnequipItem(ID actor, Base base, Value value) _CPP(noexcept)
{

}

