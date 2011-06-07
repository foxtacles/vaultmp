#include <vaultmp>

main()
{
timestamp();
print("Random text from my vaultmp script in color red.\n", 1, -1, 1);
new name[32] = "My first vaultmp server";
new map[32] = "the wasteland";
new rule[16] = "version";
new value[16] = "0.1a";
SetServerName(name);
SetServerMap(map);
SetServerRule(rule, value);
}

public OnClientAuthenticate(clientID, const name[], const pwd[])
{
	timestamp();
	printf("OnClientAuthenticate: ID: %d name: %s, pwd: %s\n", clientID, name, pwd);
	return 1;
}

public OnClientRequestGame(clientID, savegame[], len)
{
	timestamp();
	printf("OnClientRequestGame: ID: %d savegame: %s, len: %d\n", clientID, savegame, len);
	return 1;
}

public OnPlayerJoin(clientID)
{
	timestamp();
	new name[32];
	GetPlayerName(clientID, name);
	printf("OnPlayerJoin: ID: %d name: %s\n", clientID, name);
	return 1;
}

public OnPlayerDeath(clientID)
{
	timestamp();
	new name[32]; new str[64];
	GetPlayerName(clientID, name);
	strformat(str, sizeof(str), false, "OnPlayerDeath: %s died\n", name);
	print(str, 1, -1, 1);
	return 1;
}

public OnPlayerDisconnect(clientID)
{
	timestamp();
	new name[32];
	GetPlayerName(clientID, name);
	printf("OnPlayerDisconnect: ID: %d name: %s\n", clientID, name);
	return 1;
}