/* Vault-Tec Multiplayer */

// Callbacks

/* OnClientAuthenticate
 * called when a client attempts to join the server
 * return 0: kick player
 */
forward OnClientAuthenticate(clientID, const name[], const pwd[]);

/* OnClientRequestGame
 * called when a client attempts to start a game
 * set savegame string to the savegame name for the player to load (maxlength: len)
 * return 0: kick player
 */
forward OnClientRequestGame(clientID, savegame[], len);

/* OnPlayerJoin
 * called when a client is in-game (client is now a player)
 */
forward OnPlayerJoin(clientID);

// Functions

/* GetPlayerName
 * supply a buffer of at least 16 cells!
 * returns 0: invalid clientID
 */

native GetPlayerName(clientID, name[]);