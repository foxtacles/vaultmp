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

public OnClientAuthenticate(const name{}, const pwd{})
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

public OnSpawn(ID)
{

}

public OnCellChange(ID, cell)
{

}

public OnContainerItemChange(ID, base, count, Float:value)
{

}

public OnActorValueChange(ID, index, Float:value)
{

}

public OnActorBaseValueChange(ID, index, Float:value)
{

}

public OnActorAlert(ID, Bool:alerted)
{

}

public OnActorSneak(ID, Bool:sneaking)
{

}

public OnActorDeath(ID)
{

}

public OnActorEquipItem(ID, base, Float:value)
{

}

public OnActorUnequipItem(ID, base, Float:value)
{

}