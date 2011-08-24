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

public OnClientAuthenticate(client, const name[], const pwd[])
{
	printf("PAWN: client auth %d, %s, %s\n", client, name, pwd);
	return true;
}

public OnPlayerConnect(player)
{

}

public OnPlayerDisconnect(player, reason)
{
	printf("PAWN: player disconnect %d, %d\n", player, reason);
}

public OnPlayerRequestGame(player)
{
	return 0;
}

public OnPlayerSpawn(player)
{

}

public OnPlayerDeath(player)
{

}

public OnPlayerCellChange(player, cell)
{

}