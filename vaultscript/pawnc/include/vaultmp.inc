/* Vault-Tec Multiplayer */

// Constants

#define FALLOUT3                0x01
#define NEWVEGAS                (FALLOUT3 << 1)
#define OBLIVION                (NEWVEGAS << 1)
#define FALLOUT_GAMES		(FALLOUT3 | NEWVEGAS)
#define ALL_GAMES		(FALLOUT_GAMES | OBLIVION)

// Callbacks

// the PAWN equivalent to exec() is main()
forward OnClientAuthenticate(const name{}, const pwd{});
forward OnPlayerDisconnect(player, reason);
forward OnPlayerRequestGame(player);
forward OnPlayerSpawn(player);
forward OnPlayerDeath(player);
forward OnPlayerCellChange(player, cell);
forward OnPlayerValueChange(player, index, Float:value);
forward OnPlayerBaseValueChange(player, index, Float:value);
forward OnPlayerStateChange(player, index, Bool:alerted);

// Functions

native timestamp();
native CreateTimer(const func{}, interval);
native KillTimer(timer);

native SetServerName(name{});
native SetServerMap(map{});
native SetServerRule(rule{}, value{});
native GetGameCode();

native ValueToString(index, value{});
native AxisToString(index, value{});
native AnimToString(index, value{});

native Float:GetPlayerPos(player, index);
native GetPlayerPosXYZ(player, &Float:X, &Float:Y, &Float:Z);
native Float:GetPlayerAngle(player, index);
native GetPlayerAngleXYZ(player, &Float:X, &Float:Y, &Float:Z);
native Float:GetPlayerValue(player, index);
native Float:GetPlayerBaseValue(player, index);
native GetPlayerCell(player);