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
forward OnSpawn(object);
forward OnCellChange(object, cell);
forward OnActorDeath(actor);
forward OnActorValueChange(actor, index, Float:value);
forward OnActorBaseValueChange(actor, index, Float:value);
forward OnActorStateChange(actor, index, Bool:alerted);

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

native GetPos(player, &Float:X, &Float:Y, &Float:Z);
native GetAngle(player, &Float:X, &Float:Y, &Float:Z);
native GetCell(object);
native Float:GetActorValue(actor, index);
native Float:GetActorBaseValue(actor, index);