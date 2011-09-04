#include "PAWN.h"

/* AMX extension modules */
#define FLOATPOINT // floating point for console
#define AMXCONSOLE_NOIDLE // no idle state
#define AMXTIME_NOIDLE // no idle state
#define NDEBUG // no debug
#include "amx/amxcore.c"
#undef __T
#include "amx/amxfile.c"
#undef __T
#include "amx/amxcons.c"
#undef __T
#include "amx/amxstring.c"
#include "amx/amxtime.c"
#include "amx/float.c"

AMX_NATIVE_INFO PAWN::vaultmp_functions[] =
{

    {"timestamp", PAWN::vaultmp_timestamp},
    {"CreateTimer", PAWN::vaultmp_CreateTimer},
    {"KillTimer", PAWN::vaultmp_KillTimer},

    {"SetServerName", PAWN::vaultmp_SetServerName},
    {"SetServerMap", PAWN::vaultmp_SetServerMap},
    {"SetServerRule", PAWN::vaultmp_SetServerRule},
    {"GetGameCode", PAWN::vaultmp_GetGameCode},

    {"ValueToString", PAWN::vaultmp_ValueToString},
    {"AxisToString", PAWN::vaultmp_AxisToString},
    {"AnimToString", PAWN::vaultmp_AnimToString},

    {"GetPlayerPos", PAWN::vaultmp_GetPlayerPos},
    {"GetPlayerPosXYZ", PAWN::vaultmp_GetPlayerPosXYZ},
    {"GetPlayerAngle", PAWN::vaultmp_GetPlayerAngle},
    {"GetPlayerAngleXYZ", PAWN::vaultmp_GetPlayerAngleXYZ},
    {"GetPlayerValue", PAWN::vaultmp_GetPlayerValue},
    {"GetPlayerBaseValue", PAWN::vaultmp_GetPlayerBaseValue},
    {"GetPlayerCell", PAWN::vaultmp_GetPlayerCell},

    /*{"GetPlayerName", PAWN::vaultmp_GetPlayerName},
    {"GetPlayerPos", PAWN::vaultmp_GetPlayerPos},
    {"GetPlayerZAngle", PAWN::vaultmp_GetPlayerZAngle},
    {"GetPlayerHealth", PAWN::vaultmp_GetPlayerHealth},
    {"GetPlayerBaseHealth", PAWN::vaultmp_GetPlayerBaseHealth},
    {"GetPlayerCondition", PAWN::vaultmp_GetPlayerCondition},
    {"IsPlayerDead", PAWN::vaultmp_IsPlayerDead},
    {"GetPlayerMoving", PAWN::vaultmp_GetPlayerMoving},
    {"IsPlayerAlerted", PAWN::vaultmp_IsPlayerAlerted},
    {"GetPlayerCell", PAWN::vaultmp_GetPlayerCell},*/

    {0, 0}

};

int PAWN::RegisterVaultmpFunctions(AMX* amx)
{
    return PAWN::Register(amx, PAWN::vaultmp_functions, -1);
}

cell PAWN::vaultmp_timestamp(AMX* amx, const cell* params)
{
    cell i = 1;
    Utils::timestamp();
    return i;
}

cell PAWN::vaultmp_CreateTimer(AMX* amx, const cell* params)
{
    cell i = 1, interval; int len;
    cell* source;

    source = amx_Address(amx, params[1]);
    amx_StrLen(source, &len);
    char name[len + 1];

    amx_GetString(name, source, 0, UNLIMITED);

    interval = params[2];

    i = (cell) Script::CreateTimerPAWN(string(name), amx, (unsigned int) interval);

    return i;
}

cell PAWN::vaultmp_KillTimer(AMX* amx, const cell* params)
{
    cell i = 1, id;

    id = params[1];

    Script::KillTimer((NetworkID) id);

    return i;
}

cell PAWN::vaultmp_SetServerName(AMX* amx, const cell* params)
{
    cell i = 1; int len;
    cell* source;

    source = amx_Address(amx, params[1]);
    amx_StrLen(source, &len);
    char name[len + 1];

    amx_GetString(name, source, 0, UNLIMITED);

    Dedicated::SetServerName(string(name));

    return i;
}

cell PAWN::vaultmp_SetServerMap(AMX* amx, const cell* params)
{
    cell i = 1; int len;
    cell* source;

    source = amx_Address(amx, params[1]);
    amx_StrLen(source, &len);
    char map[len + 1];

    amx_GetString(map, source, 0, UNLIMITED);

    Dedicated::SetServerMap(string(map));

    return i;
}

cell PAWN::vaultmp_SetServerRule(AMX* amx, const cell* params)
{
    cell i = 1; int len, len2;
    cell* source;
    cell* source2;

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

cell PAWN::vaultmp_GetGameCode(AMX* amx, const cell* params)
{
    return (cell) Dedicated::GetGameCode();
}

cell PAWN::vaultmp_ValueToString(AMX* amx, const cell* params)
{
    cell i = 1, index;
    cell* dest;

    index = params[1];
    dest = amx_Address(amx, params[2]);

    string value = API::RetrieveValue_Reverse((unsigned char) index);

    if (!value.empty())
    {
        amx_SetString(dest, value.c_str(), 1, 0, value.length() + 1);
    }
    else
        i = 0;

    return i;
}

cell PAWN::vaultmp_AxisToString(AMX* amx, const cell* params)
{
    cell i = 1, index;
    cell* dest;

    index = params[1];
    dest = amx_Address(amx, params[2]);

    string axis = API::RetrieveAxis_Reverse((unsigned char) index);

    if (!axis.empty())
        amx_SetString(dest, axis.c_str(), 1, 0, axis.length() + 1);
    else
        i = 0;

    return i;
}

cell PAWN::vaultmp_AnimToString(AMX* amx, const cell* params)
{
    cell i = 1, index;
    cell* dest;

    index = params[1];
    dest = amx_Address(amx, params[2]);

    string anim = API::RetrieveAnim_Reverse((unsigned char) index);

    if (!anim.empty())
        amx_SetString(dest, anim.c_str(), 1, 0, anim.length() + 1);
    else
        i = 0;

    return i;
}

cell PAWN::vaultmp_GetPlayerPos(AMX* amx, const cell* params)
{
    cell i = 1, id, index;

    id = params[1];
    index = params[2];

    double value = Script::GetPlayerPos((unsigned int) id, (unsigned char) index);
    i = amx_ftoc(value);

    return i;
}

cell PAWN::vaultmp_GetPlayerPosXYZ(AMX* amx, const cell* params)
{
    cell i = 1, id;
    cell* X; cell* Y; cell* Z;

    id = params[1];
    X = amx_Address(amx, params[2]);
    Y = amx_Address(amx, params[3]);
    Z = amx_Address(amx, params[4]);

    double dX, dY, dZ;
    Script::GetPlayerPosXYZ((unsigned int) id, dX, dY, dZ);
    *X = amx_ftoc(dX);
    *Y = amx_ftoc(dY);
    *Z = amx_ftoc(dZ);

    return i;
}

cell PAWN::vaultmp_GetPlayerAngle(AMX* amx, const cell* params)
{
    cell i = 1, id, index;

    id = params[1];
    index = params[2];

    double value = Script::GetPlayerAngle((unsigned int) id, (unsigned char) index);
    i = amx_ftoc(value);

    return i;
}

cell PAWN::vaultmp_GetPlayerAngleXYZ(AMX* amx, const cell* params)
{
    cell i = 1, id;
    cell* X; cell* Y; cell* Z;

    id = params[1];
    X = amx_Address(amx, params[2]);
    Y = amx_Address(amx, params[3]);
    Z = amx_Address(amx, params[4]);

    double dX, dY, dZ;
    Script::GetPlayerAngleXYZ((unsigned int) id, dX, dY, dZ);
    *X = amx_ftoc(dX);
    *Y = amx_ftoc(dY);
    *Z = amx_ftoc(dZ);

    return i;
}

cell PAWN::vaultmp_GetPlayerValue(AMX* amx, const cell* params)
{
    cell i = 1, id, index;

    id = params[1];
    index = params[2];

    double value = Script::GetPlayerValue((unsigned int) id, (unsigned char) index);
    i = amx_ftoc(value);

    return i;
}

cell PAWN::vaultmp_GetPlayerBaseValue(AMX* amx, const cell* params)
{
    cell i = 1, id, index;

    id = params[1];
    index = params[2];

    double value = Script::GetPlayerBaseValue((unsigned int) id, (unsigned char) index);
    i = amx_ftoc(value);

    return i;
}

cell PAWN::vaultmp_GetPlayerCell(AMX* amx, const cell* params)
{
    cell i = 1, id;

    id = params[1];

    unsigned int value = Script::GetPlayerCell((unsigned int) id);
    i = (cell) value;

    return i;
}

/*cell PAWN::vaultmp_GetPlayerName(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetPlayerPos(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetPlayerZAngle(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetPlayerHealth(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetPlayerBaseHealth(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetPlayerCondition(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_IsPlayerDead(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetPlayerMoving(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_IsPlayerAlerted(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetPlayerCell(AMX* amx, const cell* params)
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
}*/

int PAWN::LoadProgram(AMX* amx, char* filename, void* memblock)
{
    return aux_LoadProgram(amx, filename, memblock);
}

int PAWN::Register(AMX* amx, const AMX_NATIVE_INFO* list, int number)
{
    return amx_Register(amx, list, number);
}

int PAWN::Exec(AMX* amx, cell* retval, int index)
{
    return amx_Exec(amx, retval, index);
}

int PAWN::FreeProgram(AMX* amx)
{
    return aux_FreeProgram(amx);
}

cell PAWN::Call(AMX* amx, const char* name, const char* argl, int buf, ...)
{
    va_list args;
    va_start(args, buf);
    cell ret = 0;

    try
    {
        int idx = 0;
        int err = 0;
        int pos = -1;
        int count = 0;

        err = amx_FindPublic(amx, name, &idx);
        if (err != AMX_ERR_NONE)
            throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err));

        vector<pair<cell*, char*> > strings;

        for (int i = 0; i < strlen(argl); i++)
        {
            switch (argl[i])
            {
            case 'i':
            {
                cell value = (cell) va_arg(args, unsigned int);
                amx_Push(amx, value);
                break;
            }
            case 'f':
            {
                double value = va_arg(args, double);
                amx_Push(amx, amx_ftoc(value));
                break;
            }
            case 's':
            {
                char* string = va_arg(args, char*);
                cell* store;
                amx_PushString(amx, &store, string, 1, 0);
                strings.push_back(pair<cell*, char*>(store, string));
                break;
            }
            }
        }

        err = amx_Exec(amx, &ret, idx);
        if (err != AMX_ERR_NONE)
            throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err));

        if (buf != 0)
        {
            for (vector<pair<cell*, char*> >::iterator it = strings.begin(); it != strings.end(); ++it)
            {
                int length;
                amx_StrLen(it->first, &length);

                if (buf >= length)
                {
                    ZeroMemory(it->second, buf);
                    amx_GetString(it->second, it->first, 0, UNLIMITED);
                }
            }
        }

        if (!strings.empty()) amx_Release(amx, strings.at(0).first);
    }
    catch (...)
    {
        va_end(args);
        throw;
    }

    return ret;
}

int PAWN::CoreInit(AMX* amx)
{
    return amx_CoreInit(amx);
}

int PAWN::ConsoleInit(AMX* amx)
{
    return amx_ConsoleInit(amx);
}

int PAWN::FloatInit(AMX* amx)
{
    return amx_FloatInit(amx);
}

int PAWN::TimeInit(AMX* amx)
{
    return amx_TimeInit(amx);
}

int PAWN::StringInit(AMX* amx)
{
    return amx_StringInit(amx);
}

int PAWN::FileInit(AMX* amx)
{
    return amx_FileInit(amx);
}
