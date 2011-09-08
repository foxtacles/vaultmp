#include <vaultmp>

forward MyTimer();

public MyTimer()
{

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

    //CreateTimer("MyTimer", 5000);
}

public OnClientAuthenticate(const name{}, const pwd{})
{
	return true;
}

public OnPlayerDisconnect(player, reason)
{
}

public OnPlayerRequestGame(player)
{
    new base = 0x00000000;

    switch (GetGameCode())
    {
    case FALLOUT3:
        base = 0x00030D82; // Carter
    case NEWVEGAS:
        base = 0x0010C0BE; // Jessup
    case OBLIVION:
        base = 0x000A3166; // Achille
    }

    return base;
}

public OnSpawn(object)
{

}

public OnCellChange(object, cell)
{

}

public OnActorDeath(actor)
{

}

public OnActorValueChange(actor, index, Float:value)
{

}

public OnActorBaseValueChange(actor, index, Float:value)
{

}

public OnActorStateChange(actor, index, Bool:alerted)
{

}