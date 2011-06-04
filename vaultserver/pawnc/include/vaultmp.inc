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

// Callbacks

/* OnClientAuthenticate
 * called when a client attempts to join the server
 * return 0: kick player
 */
forward OnClientAuthenticate(clientID, const name[], const pwd[]);

/* OnClientRequestGame
 * called when a client attempts to start a game
 * set savegame string to the savegame name for the client to load (maxlength: len)
 * return 0: kick player
 */
forward OnClientRequestGame(clientID, savegame[], len);

/* OnPlayerJoin
 * called when a client is in-game (client is now a player)
 */
forward OnPlayerJoin(clientID);

/* OnPlayerDisconnect
 * called when a player leaves the game; note that you can't use any setter functions on the player anymore
 */
forward OnPlayerDisconnect(clientID);

// Functions

/* GetPlayerName
 * supply a buffer of at least 16 cells
 * returns 0: invalid clientID
 */

native GetPlayerName(clientID, name[]);

/* GetPlayerPos
 * returns a Float referring to the position value on the specified axis (X_AXIS = X, Y_AXIS = Y, Z_AXIS = Z)
 * returns 0: invalid clientID / invalid axis parameter
 */

native Float:GetPlayerPos(clientID, axis);

/* GetPlayerZAngle
 * returns a Float referring to the Z angle
 * returns 0: invalid clientID
 */

native Float:GetPlayerZAngle(clientID);

/* GetPlayerHealth
 * returns a Float referring to the players health
 * returns 0: invalid clientID
 */

native Float:GetPlayerHealth(clientID);

/* GetPlayerBaseHealth
 * returns a Float referring to the players base health
 * returns 0: invalid clientID
 */

native Float:GetPlayerBaseHealth(clientID);

/* GetPlayerCondition
 * returns a Float referring to the players condition of the specified function (see Constants for a list)
 * returns 0: invalid clientID / invalid condition parameter
 */

native Float:GetPlayerCondition(clientID, condition);

/* IsPlayerDead
 * returns 1 if the player is dead
 * returns 0 if the player is alive
 * returns -1: invalid clientID
 */

native IsPlayerDead(clientID);

/* GetPlayerMoving
 * returns a moving state (see Constants for a list)
 * returns -1: invalid clientID
 */

native GetPlayerMoving(clientID);