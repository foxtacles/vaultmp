/* Vault-Tec Multiplayer */

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
 * returns a Float referring to the position value on the specified axis (0 = X, 1 = Y, 2 = Z)
 * returns 0: invalid clientID / invalid axis parameter
 */

native Float:GetPlayerPos(clientID, axis);