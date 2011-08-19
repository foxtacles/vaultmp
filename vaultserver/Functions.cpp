#include "Functions.h"

/*AMX_NATIVE_INFO Functions::vaultmp_functions[] = {

    {"GetPlayerName", Functions::vaultmp_GetPlayerName},
    {"GetPlayerPos", Functions::vaultmp_GetPlayerPos},
    {"GetPlayerZAngle", Functions::vaultmp_GetPlayerZAngle},
    {"GetPlayerHealth", Functions::vaultmp_GetPlayerHealth},
    {"GetPlayerBaseHealth", Functions::vaultmp_GetPlayerBaseHealth},
    {"GetPlayerCondition", Functions::vaultmp_GetPlayerCondition},
    {"IsPlayerDead", Functions::vaultmp_IsPlayerDead},
    {"GetPlayerMoving", Functions::vaultmp_GetPlayerMoving},
    {"IsPlayerAlerted", Functions::vaultmp_IsPlayerAlerted},
    {"GetPlayerCell", Functions::vaultmp_GetPlayerCell},
    {"SetServerName", Functions::vaultmp_SetServerName},
    {"SetServerMap", Functions::vaultmp_SetServerMap},
    {"SetServerRule", Functions::vaultmp_SetServerRule},
    {"GetGame", Functions::vaultmp_GetGame},
    {"timestamp", Functions::vaultmp_timestamp},
    {0, 0}

};*/

int Functions::RegisterVaultmpFunctions(AMX* amx)
{
    //return Script::Register(amx, Functions::vaultmp_functions, -1);
    return 0;
}

/*cell Functions::vaultmp_GetPlayerName(AMX* amx, const cell* params)
{
    int i = 1, len, id;
    cell* dest;

    id = (int) params[1];
    dest = amx_Address(amx, params[2]);

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        string pname = player->GetActorName();
        char name[pname.length()];
        strcpy(name, pname.c_str());

        amx_SetString(dest, name, 0, 0, strlen(name) + 1);
    }
    else
        i = 0;

    return i;
}

cell Functions::vaultmp_GetPlayerPos(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        float pos = player->GetActorPos((int) params[2]);
        return amx_ftoc(pos);
    }
    else
        i = 0;

    return i;
}

cell Functions::vaultmp_GetPlayerZAngle(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        float angle = player->GetActorAngle();
        return amx_ftoc(angle);
    }
    else
        i = 0;

    return i;
}

cell Functions::vaultmp_GetPlayerHealth(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        float health = player->GetActorHealth();
        return amx_ftoc(health);
    }
    else
        i = 0;

    return i;
}

cell Functions::vaultmp_GetPlayerBaseHealth(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        float baseHealth = player->GetActorBaseHealth();
        return amx_ftoc(baseHealth);
    }
    else
        i = 0;

    return i;
}

cell Functions::vaultmp_GetPlayerCondition(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        float cond = player->GetActorCondition((int) params[2]);
        return amx_ftoc(cond);
    }
    else
        i = 0;

    return i;
}

cell Functions::vaultmp_IsPlayerDead(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        return player->IsActorDead();
    }
    else
        i = -1;

    return i;
}

cell Functions::vaultmp_GetPlayerMoving(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        return player->GetActorMoving();
    }
    else
        i = -1;

    return i;
}

cell Functions::vaultmp_IsPlayerAlerted(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        return player->IsActorAlerted();
    }
    else
        i = -1;

    return i;
}

cell Functions::vaultmp_GetPlayerCell(AMX* amx, const cell* params)
{
    int i = 1, id;

    id = (int) params[1];

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        return player->GetActorNetworkCell();
    }
    else
        i = -1;

    return i;
}

cell Functions::vaultmp_SetServerName(AMX* amx, const cell* params)
{
    int i = 1, len;
    cell* source;

    source = amx_Address(amx, params[1]);
    amx_StrLen(source, &len);

    char name[len + 1];

    amx_GetString(name, source, 0, UNLIMITED);

    Dedicated::SetServerName(string(name));

    return i;
}

cell Functions::vaultmp_SetServerMap(AMX* amx, const cell* params)
{
    int i = 1, len;
    cell* source;

    source = amx_Address(amx, params[1]);
    amx_StrLen(source, &len);

    char map[len + 1];

    amx_GetString(map, source, 0, UNLIMITED);

    Dedicated::SetServerMap(string(map));

    return i;
}

cell Functions::vaultmp_SetServerRule(AMX* amx, const cell* params)
{
    int i = 1, len, len2;
    cell* source; cell* source2;

    source = amx_Address(amx, params[1]);
    source2 = amx_Address(amx, params[2]);
    amx_StrLen(source, &len);
    amx_StrLen(source2, &len2);

    char rule[len + 1];
    char value[len2 + 1];

    amx_GetString(rule, source, 0, UNLIMITED);
    amx_GetString(value, source2, 0, UNLIMITED);

    Dedicated::SetServerRule(string(rule), string(value));

    return i;
}

cell Functions::vaultmp_GetGame(AMX* amx, const cell* params)
{
    return (cell) Dedicated::GetGame();
}

cell Functions::vaultmp_timestamp(AMX* amx, const cell* params)
{
    int i = 1;

    Utils::timestamp();

    return i;
}
*/
