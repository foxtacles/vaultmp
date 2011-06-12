/* Vault-Tec Multiplayer */

// Constants

#define		X_AXIS	0
#define		Y_AXIS  1
#define		Z_AXIS	2

#define		COND_PERCEPTION    0
#define		COND_ENDURANCE	   1
#define		COND_LEFTATTACK    2
#define		COND_RIGHTATTACK   3
#define 	COND_LEFTMOBILITY  4
#define		COND_RIGHTMOBILITY 5

#define		MOV_IDLE	 0
#define		MOV_FASTFORWARD  1
#define		MOV_FASTBACKWARD 2
#define		MOV_FASTLEFT     3
#define 	MOV_FASTRIGHT	 4
#define		MOV_FORWARD      5
#define		MOV_BACKWARD     6
#define		MOV_LEFT         7
#define 	MOV_RIGHT	 8

// Callbacks

forward OnClientAuthenticate(clientID, const name[], const pwd[]);
forward OnClientRequestGame(clientID, savegame[], len);
forward OnPlayerJoin(clientID);
forward OnPlayerDeath(clientID);
forward OnPlayerDisconnect(clientID);

// Player functions

native GetPlayerName(clientID, name[]);
native Float:GetPlayerPos(clientID, axis);
native Float:GetPlayerZAngle(clientID);
native Float:GetPlayerHealth(clientID);
native Float:GetPlayerBaseHealth(clientID);
native Float:GetPlayerCondition(clientID, condition);
native IsPlayerDead(clientID);
native GetPlayerMoving(clientID);
native IsPlayerAlerted(clientID);

// Server info functions

native SetServerName(name[]);
native SetServerMap(map[]);
native SetServerRule(rule[], value[]);
native bool:IsNewVegas();

// Misc functions

native timestamp();