#include "Functions.h"

AMX_NATIVE_INFO Functions::vaultmp_functions[] = {

    {"GetPlayerName", Functions::vaultmp_GetPlayerName},
    {"GetPlayerPos", Functions::vaultmp_GetPlayerPos},
    {"GetPlayerZAngle", Functions::vaultmp_GetPlayerZAngle},
    {"GetPlayerHealth", Functions::vaultmp_GetPlayerHealth},
    {"GetPlayerBaseHealth", Functions::vaultmp_GetPlayerBaseHealth},
    {"GetPlayerCondition", Functions::vaultmp_GetPlayerCondition},
    {"IsPlayerDead", Functions::vaultmp_IsPlayerDead},
    {"GetPlayerMoving", Functions::vaultmp_GetPlayerMoving},
    {0, 0}

};

int Functions::RegisterVaultmpFunctions(AMX* amx)
{
    return Script::Register(amx, Functions::vaultmp_functions, -1);
}

cell Functions::vaultmp_GetPlayerName(AMX* amx, const cell* params)
{
    int i = 1, len, id;
    cell* dest;

    id = (int) params[1];
    amx_GetAddr(amx, params[2], &dest);

    RakNetGUID guid = Client::GetGUIDFromID(id);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL)
    {
        string pname = player->GetPlayerName();
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
        float pos = player->GetPlayerPos((int) params[2]);
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
        float angle = player->GetPlayerAngle();
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
        float health = player->GetPlayerHealth();
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
        float baseHealth = player->GetPlayerBaseHealth();
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
        float cond = player->GetPlayerCondition((int) params[2]);
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
        return player->IsPlayerDead();
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
        return player->GetPlayerMoving();
    }
    else
        i = -1;

    return i;
}
