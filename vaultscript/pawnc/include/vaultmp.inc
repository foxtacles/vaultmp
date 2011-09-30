/* Vault-Tec Multiplayer */

// Constants

#define FALLOUT3                0x01
#define NEWVEGAS                (FALLOUT3 << 1)
#define OBLIVION                (NEWVEGAS << 1)
#define FALLOUT_GAMES		(FALLOUT3 | NEWVEGAS)
#define ALL_GAMES		(FALLOUT_GAMES | OBLIVION)

#define MAX_PLAYER_NAME		16
#define MAX_PASSWORD_SIZE	16

// Callbacks

forward OnClientAuthenticate(const name{}, const pwd{});
forward OnPlayerDisconnect(ID, reason);
forward OnPlayerRequestGame(ID);
forward OnSpawn(ID);
forward OnCellChange(ID, cell);
forward OnActorValueChange(ID, index, Float:value);
forward OnActorBaseValueChange(ID, index, Float:value);
forward OnActorAlert(ID, Bool:alerted);
forward OnActorSneak(ID, Bool:sneaking);
forward OnActorDeath(ID);

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

native GetReference(ID);
native GetBase(ID);
native GetName(ID, name{});
native GetPos(ID, &Float:X, &Float:Y, &Float:Z);
native GetAngle(ID, &Float:X, &Float:Y, &Float:Z);
native GetCell(ID);

native Float:GetActorValue(ID, index);
native Float:GetActorBaseValue(ID, index);
native GetActorMovingAnimation(ID);
native Bool:GetActorAlerted(ID);
native Bool:GetActorSneaking(ID);
native Bool:GetActorDead(ID);
native Bool:IsActorJumping(ID);
