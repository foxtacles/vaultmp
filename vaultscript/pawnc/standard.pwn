#include <vaultmp>

forward MyTimer();

public MyTimer()
{
    printf("PAWN bar\n");
}

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
    case OBLIVION:
        SetServerMap("cyrodiil");
    }

    CreateTimer("MyTimer", 5000);
}

public OnClientAuthenticate(const name{}, const pwd{})
{
	printf("PAWN: client auth %s, %s\n", name, pwd);
	return true;
}

public OnPlayerDisconnect(player, reason)
{
	printf("PAWN: player disconnect %d, %d\n", player, reason);
}

public OnPlayerRequestGame(player)
{
    	printf("PAWN: player game %d\n", player);
    	return 0x00030D82;
}

public OnPlayerSpawn(player)
{

}

public OnPlayerDeath(player)
{

}

public OnPlayerCellChange(player, cell)
{
    	printf("PAWN: player cell %d,%x\n", player, cell);
}

public OnPlayerValueChange(player, index, Float:value)
{
	new values{64};
	ValueToString(index, values);
	printf("PAWN: player value %s -> %f\n", values, value);
}

public OnPlayerBaseValueChange(player, index, Float:value)
{
	new values{64};
	ValueToString(index, values);
	printf("PAWN: player base value %s -> %f\n", values, value);
}

public OnPlayerStateChange(player, index, Bool:alerted)
{
	new anims{64};
	AnimToString(index, anims);
	printf("PAWN: player running animation %s, alerted %d\n", anims, alerted);
}