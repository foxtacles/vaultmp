#include "../vaultscript.h"
#include <cstdio>

using namespace std;
using namespace vaultmp;

Base nuka = (Base) 0x0001519E;
Value area[] = {-12630.32, -15715.93, -12728.72, -15834.46};
IDSet player_nuka;

Result VAULTSCRIPT NukaArea()
{
	IDVector players = GetList(Type::ID_PLAYER);

	for (const ID& id : players)
	{
		if (!player_nuka.count(id) && GetActorSneaking(id))
		{
			Value X, Y, Z;
			GetPos(id, X, Y, Z);

			if (X < area[0] && X > area[2] && Y < area[1] && Y > area[3])
			{
				UIMessage(id, "You found a hidden nuka cola!");
				AddItem(id, nuka, 1, 100.0, False);
				player_nuka.insert(id);
			}
		}
	}

	return (Result) 0;
}

Void VAULTSCRIPT exec()
{
	if (GetGameCode() != Index::FALLOUT3)
	{
		printf("NukaScript is for Fallout 3 only!\n");
		terminate();
	}

	CreateTimer(&NukaArea, (Interval) 500);

	printf("NukaScript for Fallout 3 loaded\n");
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason)
{
	player_nuka.erase(player);
}

Void VAULTSCRIPT OnSpawn(ID object)
{
	player_nuka.erase(object);
}
