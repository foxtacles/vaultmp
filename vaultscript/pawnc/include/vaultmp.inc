/* Vault-Tec Multiplayer */

// Constants

#define FALLOUT3                0x01
#define NEWVEGAS                0x02
#define OBLIVION                0x04

// Callbacks

// the PAWN equivalent to exec() is main()
forward OnClientAuthenticate(const name[], const pwd[]);
forward OnPlayerConnect(player);
forward OnPlayerDisconnect(player, reason);
forward OnPlayerRequestGame(player);
forward OnPlayerSpawn(player);
forward OnPlayerDeath(player);
forward OnPlayerCellChange(player, cell);

// Functions

native timestamp();
native SetServerName(name[]);
native SetServerMap(map[]);
native SetServerRule(rule[], value[]);
native GetGameCode();