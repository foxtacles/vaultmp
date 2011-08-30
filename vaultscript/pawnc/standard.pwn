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
    case OBLIVION:
        SetServerMap("cyrodiil");
    }
}

public OnClientAuthenticate(const name[], const pwd[])
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

public OnPlayerValueChange(player, Bool:base, index, Float:value)
{
	new values[64];
	ValueToString(index, values);
	printf("PAWN: player value %s -> %f\n", values, value);
}
