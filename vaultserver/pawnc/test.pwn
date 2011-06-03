#include <vaultmp>

main()
{
print("Text aus dem ersten VaultScript ueberhaupt.\n", 1, -1, 1);
new Float: asdf;
asdf = 0.5666;
printf("float test: %f\n", asdf);
new blub[32] = "asdf";
if (strcmp(blub, "asdf") == 0)
	print("string test: asdf!\n");
}

public OnClientAuthenticate(clientID, const name[], const pwd[])
{
	printf("OnClientAuthenticate: ID: %d name: %s, pwd: %s\n", clientID, name, pwd);
	return 1;
}

public OnClientRequestGame(clientID, savegame[], len)
{
	printf("OnClientRequestGame: ID: %d savegame: %s, len: %d\n", clientID, savegame, len);
	return 1;
}

public OnPlayerJoin(clientID)
{
	new name[32];
	GetPlayerName(clientID, name);
	printf("OnPlayerJoin: ID: %d name: %s\n", clientID, name);
	return 1;
}

public OnPlayerDisconnect(clientID)
{
	new name[32];
	GetPlayerName(clientID, name);
	printf("OnPlayerDisconnect: ID: %d name: %s\n", clientID, name);
	return 1;
}