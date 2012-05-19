#include "vaultscript.h"
#include <cstdio>

using namespace vaultmp;

Result bla(int, ID , int, IDVector*) _CPP(noexcept) {
	return Result();
}

Void VAULTSCRIPT exec() _CPP(noexcept)
{
	IDVector* p = NULL;
	CreateTimer<int, ID, int, IDVector*>(bla, (Interval) 123, 123, (ID)123, 123, p);

	std::printf("My first C++ vaultscript <3\n");
	_SetServerName("vaultmp 0.1a server");
	_SetServerRule("website", "vaultmp.com");

	switch (_GetGameCode())
	{
		case Index::FALLOUT3:
			_SetServerMap("the wasteland");
			break;

		case Index::NEWVEGAS:
			_SetServerMap("mojave desert");
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
	throw std::exception();
	Base base = (Base) 0x00000000;

	switch (_GetGameCode())
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
	if (_IsPlayer(object))
	{
		//_UIMessage(object, String("Hello, ") + _GetName(object) + "!");

		Base pipboy = (Base) 0x00015038;

		if (_GetContainerItemCount(object, pipboy) == 0)
		{
			_AddItem(object, pipboy, 1, 100.0, True);
			_EquipItem(object, pipboy, True, True);
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

