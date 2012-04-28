#include "../vaultscript.h"
#include <cstdio>

using namespace std;
using namespace vaultmp;

Base _9mm = (Base) 0x000E3778;
Value spot[] = {-102.76, -398.82, 3458.55};
Value angle[] = {93.0, 134.0};
IDSet player_barrel;

Result VAULTSCRIPT BarrelSpot()
{
	IDVector players = GetList(Type::ID_PLAYER);

	for (const ID& id : players)
	{
		if (!player_barrel.count(id) && IsNearPoint(id, spot[0], spot[1], spot[2], 30.0))
		{
			Value X, Y, Z;
			GetAngle(id, X, Y, Z);

			if (Z > angle[0] && Z < angle[1])
			{
				UIMessage(id, "You found a broken 9mm pistol at the bottom of the barrel!");
				AddItem(id, _9mm, 1, 0.0, False);
				player_barrel.insert(id);
			}
		}
	}

	return (Result) 0;
}

Void VAULTSCRIPT exec()
{
	if (GetGameCode() != Index::NEWVEGAS)
	{
		printf("BarrelScript is for Fallout: New Vegas only!\n");
		terminate();
	}

	CreateTimer(&BarrelSpot, (Interval) 500);

	printf("BarrelScript for Fallout: New Vegas loaded\n");
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason)
{
	player_barrel.erase(player);
}

Void VAULTSCRIPT OnSpawn(ID object)
{
	player_barrel.erase(object);
}
