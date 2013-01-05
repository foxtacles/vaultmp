#include "PAWN.h"
#include "Script.h"

using namespace std;
using namespace RakNet;

extern "C" {
	int AMXAPI amx_CoreInit(AMX* amx);
	int AMXAPI amx_ConsoleInit(AMX* amx);
	int AMXAPI amx_FloatInit(AMX* amx);
	int AMXAPI amx_TimeInit(AMX* amx);
	int AMXAPI amx_StringInit(AMX* amx);
	int AMXAPI amx_FileInit(AMX* amx);
}

AMX_NATIVE_INFO PAWN::vaultmp_functions[] =
{
	{"timestamp", PAWN::vaultmp_timestamp},
	{"CreateTimer", PAWN::vaultmp_CreateTimer},
	{"CreateTimerEx", PAWN::vaultmp_CreateTimerEx},
	{"KillTimer", PAWN::vaultmp_KillTimer},
	{"MakePublic", PAWN::vaultmp_MakePublic},
	{"CallPublic", PAWN::vaultmp_CallPublic},

	{"SetServerName", PAWN::vaultmp_SetServerName},
	{"SetServerMap", PAWN::vaultmp_SetServerMap},
	{"SetServerRule", PAWN::vaultmp_SetServerRule},
	{"GetGameCode", PAWN::vaultmp_GetGameCode},
	{"GetMaximumPlayers", PAWN::vaultmp_GetMaximumPlayers},
	{"GetCurrentPlayers", PAWN::vaultmp_GetCurrentPlayers},

	{"ValueToString", PAWN::vaultmp_ValueToString},
	{"AxisToString", PAWN::vaultmp_AxisToString},
	{"AnimToString", PAWN::vaultmp_AnimToString},
	{"BaseToString", PAWN::vaultmp_BaseToString},

	{"UIMessage", PAWN::vaultmp_UIMessage},
	{"ChatMessage", PAWN::vaultmp_ChatMessage},
	{"SetRespawn", PAWN::vaultmp_SetRespawn},
	{"SetSpawnCell", PAWN::vaultmp_SetSpawnCell},
	{"SetGameWeather", PAWN::vaultmp_SetGameWeather},
	{"SetGameTime", PAWN::vaultmp_SetGameTime},
	{"SetGameYear", PAWN::vaultmp_SetGameYear},
	{"SetGameMonth", PAWN::vaultmp_SetGameMonth},
	{"SetGameDay", PAWN::vaultmp_SetGameDay},
	{"SetGameHour", PAWN::vaultmp_SetGameHour},
	{"SetTimeScale", PAWN::vaultmp_SetTimeScale},
	{"IsValid", PAWN::vaultmp_IsValid},
	{"IsObject", PAWN::vaultmp_IsObject},
	{"IsItem", PAWN::vaultmp_IsItem},
	{"IsContainer", PAWN::vaultmp_IsContainer},
	{"IsActor", PAWN::vaultmp_IsActor},
	{"IsPlayer", PAWN::vaultmp_IsPlayer},
	{"IsCell", PAWN::vaultmp_IsCell},
	{"IsInterior", PAWN::vaultmp_IsInterior},
	{"GetType", PAWN::vaultmp_GetType},
	{"GetConnection", PAWN::vaultmp_GetConnection},
	{"GetCount", PAWN::vaultmp_GetCount},
	{"GetList", PAWN::vaultmp_GetList},
	{"GetGameWeather", PAWN::vaultmp_GetGameWeather},
	{"GetGameTime", PAWN::vaultmp_GetGameTime},
	{"GetGameYear", PAWN::vaultmp_GetGameYear},
	{"GetGameMonth", PAWN::vaultmp_GetGameMonth},
	{"GetGameDay", PAWN::vaultmp_GetGameDay},
	{"GetGameHour", PAWN::vaultmp_GetGameHour},
	{"GetTimeScale", PAWN::vaultmp_GetTimeScale},

	{"GetReference", PAWN::vaultmp_GetReference},
	{"GetBase", PAWN::vaultmp_GetBase},
	{"GetName", PAWN::vaultmp_GetName},
	{"GetPos", PAWN::vaultmp_GetPos},
	{"GetAngle", PAWN::vaultmp_GetAngle},
	{"GetCell", PAWN::vaultmp_GetCell},
	{"IsNearPoint", PAWN::vaultmp_IsNearPoint},
	{"GetItemContainer", PAWN::vaultmp_GetItemContainer},
	{"GetItemCount", PAWN::vaultmp_GetItemCount},
	{"GetItemCondition", PAWN::vaultmp_GetItemCondition},
	{"GetItemEquipped", PAWN::vaultmp_GetItemEquipped},
	{"GetItemSilent", PAWN::vaultmp_GetItemSilent},
	{"GetItemStick", PAWN::vaultmp_GetItemStick},
	{"GetContainerItemCount", PAWN::vaultmp_GetContainerItemCount},
	{"GetActorValue", PAWN::vaultmp_GetActorValue},
	{"GetActorBaseValue", PAWN::vaultmp_GetActorBaseValue},
	{"GetActorIdleAnimation", PAWN::vaultmp_GetActorIdleAnimation},
	{"GetActorMovingAnimation", PAWN::vaultmp_GetActorMovingAnimation},
	{"GetActorWeaponAnimation", PAWN::vaultmp_GetActorWeaponAnimation},
	{"GetActorAlerted", PAWN::vaultmp_GetActorAlerted},
	{"GetActorSneaking", PAWN::vaultmp_GetActorSneaking},
	{"GetActorDead", PAWN::vaultmp_GetActorDead},
	{"GetActorBaseRace", PAWN::vaultmp_GetActorBaseRace},
	{"GetActorBaseSex", PAWN::vaultmp_GetActorBaseSex},
	{"IsActorJumping", PAWN::vaultmp_IsActorJumping},
	{"GetPlayerRespawn", PAWN::vaultmp_GetPlayerRespawn},
	{"GetPlayerSpawnCell", PAWN::vaultmp_GetPlayerSpawnCell},

	{"DestroyObject", PAWN::vaultmp_DestroyObject},
	{"SetPos", PAWN::vaultmp_SetPos},
	{"SetAngle", PAWN::vaultmp_SetAngle},
	{"SetCell", PAWN::vaultmp_SetCell},
	{"CreateItem", PAWN::vaultmp_CreateItem},
	{"SetItemCount", PAWN::vaultmp_SetItemCount},
	{"SetItemCondition", PAWN::vaultmp_SetItemCondition},
	{"CreateContainer", PAWN::vaultmp_CreateContainer},
	{"AddItem", PAWN::vaultmp_AddItem},
	{"RemoveItem", PAWN::vaultmp_RemoveItem},
	{"RemoveAllItems", PAWN::vaultmp_RemoveAllItems},
	{"CreateActor", PAWN::vaultmp_CreateActor},
	{"SetActorValue", PAWN::vaultmp_SetActorValue},
	{"SetActorBaseValue", PAWN::vaultmp_SetActorBaseValue},
	{"EquipItem", PAWN::vaultmp_EquipItem},
	{"UnequipItem", PAWN::vaultmp_UnequipItem},
	{"PlayIdle", PAWN::vaultmp_PlayIdle},
	{"KillActor", PAWN::vaultmp_KillActor},
	{"SetActorBaseRace", PAWN::vaultmp_SetActorBaseRace},
	{"AgeActorBaseRace", PAWN::vaultmp_AgeActorBaseRace},
	{"SetActorBaseSex", PAWN::vaultmp_SetActorBaseSex},
	{"SetPlayerRespawn", PAWN::vaultmp_SetPlayerRespawn},
	{"SetPlayerSpawnCell", PAWN::vaultmp_SetPlayerSpawnCell},

	{0, 0}
};

int PAWN::RegisterVaultmpFunctions(AMX* amx)
{
	return PAWN::Register(amx, PAWN::vaultmp_functions, -1);
}

cell PAWN::vaultmp_timestamp(AMX*, const cell*)
{
	Utils::timestamp();
	return 1;
}

cell PAWN::vaultmp_CreateTimer(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);

	vector<char> name;
	name.reserve(len + 1);

	amx_GetString(&name[0], source, 0, UNLIMITED);

	return Script::CreateTimerPAWN(&name[0], amx, params[2]);
}

cell PAWN::vaultmp_CreateTimerEx(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);

	vector<char> name;
	name.reserve(len + 1);

	amx_GetString(&name[0], source, 0, UNLIMITED);

	source = amx_Address(amx, params[3]);
	amx_StrLen(source, &len);

	vector<char> def;
	def.reserve(len + 1);

	amx_GetString(&def[0], source, 0, UNLIMITED);

	vector<boost::any> args;
	unsigned int count = (params[0] / sizeof(cell)) - 3;

	if (count != len)
		throw VaultException("Script call: Number of arguments does not match definition");

	for (unsigned int i = 0; i < count; ++i)
	{
		cell* data = amx_Address(amx, params[i + 4]);

		switch (def[i])
		{
			case 'i':
			{
				args.emplace_back((unsigned int) *data);
				break;
			}

			case 'l':
			{
				args.emplace_back((unsigned long long) *data);
				break;
			}

			case 'f':
			{
				args.emplace_back((double) amx_ctof(*data));
				break;
			}

			case 's':
			{
				amx_StrLen(data, &len);
				vector<char> str;
				str.reserve(len + 1);
				amx_GetString(&str[0], data, 0, UNLIMITED);
				args.emplace_back(string(&str[0]));
				break;
			}

			default:
				throw VaultException("PAWN call: Unknown argument identifier %02X", def[i]);
		}
	}

	return Script::CreateTimerPAWNEx(&name[0], amx, params[2], &def[0], args);
}

cell PAWN::vaultmp_KillTimer(AMX*, const cell* params)
{
	Script::KillTimer(params[1]);
	return 1;
}

cell PAWN::vaultmp_MakePublic(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	vector<char> real;
	real.reserve(len + 1);

	amx_GetString(&real[0], source, 0, UNLIMITED);

	source = amx_Address(amx, params[2]);
	amx_StrLen(source, &len);
	vector<char> name;
	name.reserve(len + 1);

	amx_GetString(&name[0], source, 0, UNLIMITED);

	source = amx_Address(amx, params[3]);
	amx_StrLen(source, &len);
	vector<char> def;
	def.reserve(len + 1);

	amx_GetString(&def[0], source, 0, UNLIMITED);

	Script::MakePublicPAWN(&real[0], amx, &name[0], &def[0]);

	return 1;
}

cell PAWN::vaultmp_CallPublic(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	vector<char> name;
	name.reserve(len + 1);

	amx_GetString(&name[0], source, 0, UNLIMITED);

	string def = Public::GetDefinition(&name[0]);

	vector<boost::any> args;
	unsigned int count = (params[0] / sizeof(cell)) - 1;

	if (count != def.length())
		throw VaultException("Script call: Number of arguments does not match definition");

	for (unsigned int i = 0; i < count; ++i)
	{
		cell* data = amx_Address(amx, params[i + 2]);

		switch (def[i])
		{
			case 'i':
			{
				args.emplace_back((unsigned int) *data);
				break;
			}

			case 'l':
			{
				args.emplace_back((unsigned long long) *data);
				break;
			}

			case 'f':
			{
				args.emplace_back((double) amx_ctof(*data));
				break;
			}

			case 'p':
			{
				args.emplace_back((void*) data);
				break;
			}

			case 's':
			{
				amx_StrLen(data, &len);
				vector<char> str;
				str.reserve(len + 1);
				amx_GetString(&str[0], data, 0, UNLIMITED);
				args.emplace_back(string(&str[0]));
				break;
			}

			default:
				throw VaultException("PAWN call: Unknown argument identifier %02X", def[i]);
		}
	}

	return Script::CallPublicPAWN(&name[0], args);
}

cell PAWN::vaultmp_SetServerName(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	vector<char> name;
	name.reserve(len + 1);

	amx_GetString(&name[0], source, 0, UNLIMITED);

	Dedicated::SetServerName(&name[0]);

	return 1;
}

cell PAWN::vaultmp_SetServerMap(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	vector<char> map;
	map.reserve(len + 1);

	amx_GetString(&map[0], source, 0, UNLIMITED);

	Dedicated::SetServerMap(&map[0]);

	return 1;
}

cell PAWN::vaultmp_SetServerRule(AMX* amx, const cell* params)
{
	int len, len2;
	cell* source;
	cell* source2;

	source = amx_Address(amx, params[1]);
	source2 = amx_Address(amx, params[2]);
	amx_StrLen(source, &len);
	amx_StrLen(source2, &len2);

	vector<char> rule;
	rule.reserve(len + 1);
	vector<char> value;
	value.reserve(len2 + 1);

	amx_GetString(&rule[0], source, 0, UNLIMITED);
	amx_GetString(&value[0], source2, 0, UNLIMITED);

	Dedicated::SetServerRule(&rule[0], &value[0]);

	return 1;
}

cell PAWN::vaultmp_GetGameCode(AMX*, const cell*)
{
	return Dedicated::GetGameCode();
}

cell PAWN::vaultmp_GetMaximumPlayers(AMX*, const cell*)
{
	return Dedicated::GetMaximumPlayers();
}

cell PAWN::vaultmp_GetCurrentPlayers(AMX*, const cell*)
{
	return Dedicated::GetCurrentPlayers();
}

cell PAWN::vaultmp_ValueToString(AMX* amx, const cell* params)
{
	string value = API::RetrieveValue_Reverse(params[1]);

	if (!value.empty())
	{
		cell* dest = amx_Address(amx, params[2]);
		amx_SetString(dest, value.c_str(), 1, 0, value.length() + 1);
	}
	else
		return 0;

	return 1;
}

cell PAWN::vaultmp_AxisToString(AMX* amx, const cell* params)
{
	string axis = API::RetrieveAxis_Reverse(params[1]);

	if (!axis.empty())
	{
		cell* dest = amx_Address(amx, params[2]);
		amx_SetString(dest, axis.c_str(), 1, 0, axis.length() + 1);
	}
	else
		return 0;

	return 1;
}

cell PAWN::vaultmp_AnimToString(AMX* amx, const cell* params)
{
	string anim = API::RetrieveAnim_Reverse(params[1]);

	if (!anim.empty())
	{
		cell* dest = amx_Address(amx, params[2]);
		amx_SetString(dest, anim.c_str(), 1, 0, anim.length() + 1);
	}
	else
		return 0;

	return 1;
}

cell PAWN::vaultmp_BaseToString(AMX* amx, const cell* params)
{
	string base = Script::BaseToString(params[1]);

	if (!base.empty())
	{
		cell* dest = amx_Address(amx, params[2]);
		amx_SetString(dest, base.c_str(), 1, 0, base.length() + 1);
	}
	else
		return 0;

	return 1;
}

cell PAWN::vaultmp_UIMessage(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[2]);
	amx_StrLen(source, &len);
	vector<char> message;
	message.reserve(len + 1);

	amx_GetString(&message[0], source, 0, UNLIMITED);

	return Script::UIMessage(params[1], &message[0]);
}

cell PAWN::vaultmp_ChatMessage(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[2]);
	amx_StrLen(source, &len);
	vector<char> message;
	message.reserve(len + 1);

	amx_GetString(&message[0], source, 0, UNLIMITED);

	return Script::ChatMessage(params[1], &message[0]);
}

cell PAWN::vaultmp_SetRespawn(AMX*, const cell* params)
{
	Script::SetRespawn(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetSpawnCell(AMX*, const cell* params)
{
	Script::SetSpawnCell(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetGameWeather(AMX*, const cell* params)
{
	Script::SetGameWeather(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetGameTime(AMX*, const cell* params)
{
	Script::SetGameTime(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetGameYear(AMX*, const cell* params)
{
	Script::SetGameYear(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetGameMonth(AMX*, const cell* params)
{
	Script::SetGameMonth(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetGameDay(AMX*, const cell* params)
{
	Script::SetGameDay(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetGameHour(AMX*, const cell* params)
{
	Script::SetGameHour(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetTimeScale(AMX*, const cell* params)
{
	Script::SetTimeScale(amx_ctof(params[1]));
	return 1;
}

cell PAWN::vaultmp_IsValid(AMX*, const cell* params)
{
	return Script::IsValid(params[1]);
}

cell PAWN::vaultmp_IsObject(AMX*, const cell* params)
{
	return Script::IsObject(params[1]);
}

cell PAWN::vaultmp_IsItem(AMX*, const cell* params)
{
	return Script::IsItem(params[1]);
}

cell PAWN::vaultmp_IsContainer(AMX*, const cell* params)
{
	return Script::IsContainer(params[1]);
}

cell PAWN::vaultmp_IsActor(AMX*, const cell* params)
{
	return Script::IsActor(params[1]);
}

cell PAWN::vaultmp_IsPlayer(AMX*, const cell* params)
{
	return Script::IsPlayer(params[1]);
}

cell PAWN::vaultmp_IsCell(AMX*, const cell* params)
{
	return DB::Record::IsValidCell(params[1]);
}

cell PAWN::vaultmp_IsInterior(AMX*, const cell* params)
{
	return Script::IsInterior(params[1]);
}

cell PAWN::vaultmp_GetType(AMX*, const cell* params)
{
	return GameFactory::GetType(static_cast<NetworkID>(params[1]));
}

cell PAWN::vaultmp_GetConnection(AMX*, const cell* params)
{
	return Script::GetConnection(params[1]);
}

cell PAWN::vaultmp_GetCount(AMX*, const cell* params)
{
	return GameFactory::GetObjectCount(params[1]);
}

cell PAWN::vaultmp_GetList(AMX* amx, const cell* params)
{
	vector<NetworkID> reference = GameFactory::GetIDObjectTypes(params[1]);
	cell* dest = amx_Address(amx, params[2]);
	unsigned int i = 0;

	for (const NetworkID& id : reference)
	{
		dest[i] = id;
		++i;
	}

	return reference.size();
}

cell PAWN::vaultmp_GetGameWeather(AMX*, const cell*)
{
	return Script::GetGameWeather();
}

cell PAWN::vaultmp_GetGameTime(AMX*, const cell*)
{
	return Script::GetGameTime();
}

cell PAWN::vaultmp_GetGameYear(AMX*, const cell*)
{
	return Script::GetGameYear();
}

cell PAWN::vaultmp_GetGameMonth(AMX*, const cell*)
{
	return Script::GetGameMonth();
}

cell PAWN::vaultmp_GetGameDay(AMX*, const cell*)
{
	return Script::GetGameDay();
}

cell PAWN::vaultmp_GetGameHour(AMX*, const cell*)
{
	return Script::GetGameHour();
}

cell PAWN::vaultmp_GetTimeScale(AMX*, const cell*)
{
	double value = Script::GetTimeScale();
	return amx_ftoc(value);
}

cell PAWN::vaultmp_GetReference(AMX*, const cell* params)
{
	return Script::GetReference(params[1]);
}

cell PAWN::vaultmp_GetBase(AMX*, const cell* params)
{
	return Script::GetBase(params[1]);
}

cell PAWN::vaultmp_GetName(AMX* amx, const cell* params)
{
	string name = Script::GetName(params[1]);

	if (!name.empty())
	{
		cell* dest = amx_Address(amx, params[2]);
		amx_SetString(dest, name.c_str(), 1, 0, name.length() + 1);
	}
	else
		return 0;

	return 1;
}

cell PAWN::vaultmp_GetPos(AMX* amx, const cell* params)
{
	cell* X, *Y, *Z;

	X = amx_Address(amx, params[2]);
	Y = amx_Address(amx, params[3]);
	Z = amx_Address(amx, params[4]);

	double dX, dY, dZ;

	Script::GetPos(params[1], &dX, &dY, &dZ);
	*X = amx_ftoc(dX);
	*Y = amx_ftoc(dY);
	*Z = amx_ftoc(dZ);

	return 1;
}

cell PAWN::vaultmp_GetAngle(AMX* amx, const cell* params)
{
	cell* X, *Y, *Z;

	X = amx_Address(amx, params[2]);
	Y = amx_Address(amx, params[3]);
	Z = amx_Address(amx, params[4]);

	double dX, dY, dZ;

	Script::GetAngle(params[1], &dX, &dY, &dZ);
	*X = amx_ftoc(dX);
	*Y = amx_ftoc(dY);
	*Z = amx_ftoc(dZ);

	return 1;
}

cell PAWN::vaultmp_GetCell(AMX*, const cell* params)
{
	return Script::GetCell(params[1]);
}

cell PAWN::vaultmp_IsNearPoint(AMX*, const cell* params)
{
	return Script::IsNearPoint(params[1], amx_ctof(params[2]), amx_ctof(params[3]), amx_ctof(params[4]), amx_ctof(params[5]));
}

cell PAWN::vaultmp_GetItemContainer(AMX*, const cell* params)
{
	return Script::GetItemContainer(params[1]);
}

cell PAWN::vaultmp_GetItemCount(AMX*, const cell* params)
{
	return Script::GetItemCount(params[1]);
}

cell PAWN::vaultmp_GetItemCondition(AMX*, const cell* params)
{
	double value = Script::GetItemCondition(params[1]);
	return amx_ftoc(value);
}

cell PAWN::vaultmp_GetItemEquipped(AMX*, const cell* params)
{
	return Script::GetItemEquipped(params[1]);
}

cell PAWN::vaultmp_GetItemSilent(AMX*, const cell* params)
{
	return Script::GetItemSilent(params[1]);
}

cell PAWN::vaultmp_GetItemStick(AMX*, const cell* params)
{
	return Script::GetItemStick(params[1]);
}

cell PAWN::vaultmp_GetContainerItemCount(AMX*, const cell* params)
{
	return Script::GetContainerItemCount(params[1], params[2]);
}

cell PAWN::vaultmp_GetActorValue(AMX*, const cell* params)
{
	double value = Script::GetActorValue(params[1], params[2]);
	return amx_ftoc(value);
}

cell PAWN::vaultmp_GetActorBaseValue(AMX*, const cell* params)
{
	double value = Script::GetActorBaseValue(params[1], params[2]);
	return amx_ftoc(value);
}

cell PAWN::vaultmp_GetActorIdleAnimation(AMX*, const cell* params)
{
	return Script::GetActorIdleAnimation(params[1]);
}

cell PAWN::vaultmp_GetActorMovingAnimation(AMX*, const cell* params)
{
	return Script::GetActorMovingAnimation(params[1]);
}

cell PAWN::vaultmp_GetActorWeaponAnimation(AMX*, const cell* params)
{
	return Script::GetActorWeaponAnimation(params[1]);
}

cell PAWN::vaultmp_GetActorAlerted(AMX*, const cell* params)
{
	return Script::GetActorAlerted(params[1]);
}

cell PAWN::vaultmp_GetActorSneaking(AMX*, const cell* params)
{
	return Script::GetActorSneaking(params[1]);
}

cell PAWN::vaultmp_GetActorDead(AMX*, const cell* params)
{
	return Script::GetActorDead(params[1]);
}

cell PAWN::vaultmp_GetActorBaseRace(AMX*, const cell* params)
{
	return Script::GetActorBaseRace(params[1]);
}

cell PAWN::vaultmp_GetActorBaseSex(AMX*, const cell* params)
{
	return Script::GetActorBaseSex(params[1]);
}

cell PAWN::vaultmp_IsActorJumping(AMX*, const cell* params)
{
	return Script::IsActorJumping(params[1]);
}

cell PAWN::vaultmp_GetPlayerRespawn(AMX*, const cell* params)
{
	return Script::GetPlayerRespawn(params[1]);
}

cell PAWN::vaultmp_GetPlayerSpawnCell(AMX*, const cell* params)
{
	return Script::GetPlayerSpawnCell(params[1]);
}

cell PAWN::vaultmp_DestroyObject(AMX*, const cell* params)
{
	return Script::DestroyObject(params[1]);
}

cell PAWN::vaultmp_SetPos(AMX*, const cell* params)
{
	return Script::SetPos(params[1], amx_ctof(params[2]), amx_ctof(params[3]), amx_ctof(params[4]));
}

cell PAWN::vaultmp_SetAngle(AMX*, const cell* params)
{
	return Script::SetAngle(params[1], amx_ctof(params[2]), amx_ctof(params[3]), amx_ctof(params[4]));
}

cell PAWN::vaultmp_SetCell(AMX*, const cell* params)
{
	return Script::SetCell(params[1], params[2], amx_ctof(params[3]), amx_ctof(params[4]), amx_ctof(params[5]));
}

cell PAWN::vaultmp_CreateItem(AMX*, const cell* params)
{
	return Script::CreateItem(params[1], params[2], params[3], amx_ctof(params[4]), amx_ctof(params[5]), amx_ctof(params[6]));
}

cell PAWN::vaultmp_SetItemCount(AMX*, const cell* params)
{
	return Script::SetItemCount(params[1], params[2]);
}

cell PAWN::vaultmp_SetItemCondition(AMX*, const cell* params)
{
	return Script::SetItemCondition(params[1], amx_ctof(params[2]));
}

cell PAWN::vaultmp_CreateContainer(AMX*, const cell* params)
{
	return Script::CreateContainer(params[1], params[2], params[3], amx_ctof(params[4]), amx_ctof(params[5]), amx_ctof(params[6]));
}

cell PAWN::vaultmp_AddItem(AMX*, const cell* params)
{
	return Script::AddItem(params[1], params[2], params[3], amx_ctof(params[4]), params[5]);
}

cell PAWN::vaultmp_RemoveItem(AMX*, const cell* params)
{
	return Script::RemoveItem(params[1], params[2], params[3], params[4]);
}

cell PAWN::vaultmp_RemoveAllItems(AMX*, const cell* params)
{
	Script::RemoveAllItems(params[1]);
	return 1;
}

cell PAWN::vaultmp_CreateActor(AMX*, const cell* params)
{
	return Script::CreateActor(params[1], params[2], params[3], amx_ctof(params[4]), amx_ctof(params[5]), amx_ctof(params[6]));
}

cell PAWN::vaultmp_SetActorValue(AMX*, const cell* params)
{
	Script::SetActorValue(params[1], params[2], amx_ctof(params[3]));
	return 1;
}

cell PAWN::vaultmp_SetActorBaseValue(AMX*, const cell* params)
{
	Script::SetActorBaseValue(params[1], params[2], amx_ctof(params[3]));
	return 1;
}

cell PAWN::vaultmp_EquipItem(AMX*, const cell* params)
{
	return Script::EquipItem(params[1], params[2], params[3], params[4]);
}

cell PAWN::vaultmp_UnequipItem(AMX*, const cell* params)
{
	return Script::UnequipItem(params[1], params[2], params[3], params[4]);
}

cell PAWN::vaultmp_PlayIdle(AMX*, const cell* params)
{
	return Script::PlayIdle(params[1], params[2]);
}

cell PAWN::vaultmp_KillActor(AMX*, const cell* params)
{
	Script::KillActor(params[1], params[2], params[3]);
	return 1;
}

cell PAWN::vaultmp_SetActorBaseRace(AMX*, const cell* params)
{
	return Script::SetActorBaseRace(params[1], params[2]);
}

cell PAWN::vaultmp_AgeActorBaseRace(AMX*, const cell* params)
{
	return Script::AgeActorBaseRace(params[1], params[2]);
}

cell PAWN::vaultmp_SetActorBaseSex(AMX*, const cell* params)
{
	return Script::SetActorBaseSex(params[1], params[2]);
}

cell PAWN::vaultmp_SetPlayerRespawn(AMX*, const cell* params)
{
	Script::SetPlayerRespawn(params[1], params[2]);
	return 1;
}

cell PAWN::vaultmp_SetPlayerSpawnCell(AMX*, const cell* params)
{
	Script::SetPlayerSpawnCell(params[1], params[2]);
	return 1;
}

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

bool PAWN::IsCallbackPresent(AMX* amx, const char* name)
{
	int idx = 0;
	int err = 0;
	err = amx_FindPublic(amx, name, &idx);
	return (err == AMX_ERR_NONE);
}

cell PAWN::Call(AMX* amx, const char* name, const char* argl, int buf, ...)
{
	va_list args;
	va_start(args, buf);
	cell ret = 0;
	vector<pair<cell*, char*> > strings;

	try
	{
		int idx = 0;
		int err = 0;

		err = amx_FindPublic(amx, name, &idx);

		if (err != AMX_ERR_NONE)
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err));

		unsigned int len = strlen(argl);

		for (unsigned int i = 0; i < len; i++)
		{
			switch (argl[i])
			{
				case 'i':
				{
					cell value = (cell) va_arg(args, unsigned int);
					amx_Push(amx, value);
					break;
				}

				case 'l':
				{
					cell value = (cell) va_arg(args, unsigned long long);
					amx_Push(amx, value);
					break;
				}

				case 'f':
				{
					double value = va_arg(args, double);
					amx_Push(amx, amx_ftoc(value));
					break;
				}

				case 'p':
				{
					cell value = (cell) va_arg(args, void*);
					amx_Push(amx, value);
					break;
				}

				case 's':
				{
					char* string = va_arg(args, char*);
					cell* store;
					amx_PushString(amx, &store, string, 1, 0);
					strings.emplace_back(store, string);
					break;
				}

				default:
					throw VaultException("PAWN call: Unknown argument identifier %02X", argl[i]);
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

		if (!strings.empty())
			amx_Release(amx, strings[0].first);
	}

	catch (...)
	{
		va_end(args);

		if (!strings.empty())
			amx_Release(amx, strings[0].first);

		throw;
	}

	return ret;
}

cell PAWN::Call(AMX* amx, const char* name, const char* argl, const vector<boost::any>& args)
{
	cell ret = 0;
	cell* str = nullptr;

	try
	{
		int idx = 0;
		int err = 0;

		err = amx_FindPublic(amx, name, &idx);

		if (err != AMX_ERR_NONE)
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err));

		for (int i = strlen(argl) - 1; i >= 0; i--)
		{
			switch (argl[i])
			{
				case 'i':
				{
					cell value = (cell) boost::any_cast<unsigned int>(args.at(i));
					amx_Push(amx, value);
					break;
				}

				case 'l':
				{
					cell value = (cell) boost::any_cast<unsigned long long>(args.at(i));
					amx_Push(amx, value);
					break;
				}

				case 'f':
				{
					double value = boost::any_cast<double>(args.at(i));
					amx_Push(amx, amx_ftoc(value));
					break;
				}

				case 'p':
				{
					cell value = (cell) boost::any_cast<void*>(args.at(i));
					amx_Push(amx, value);
					break;
				}

				case 's':
				{
					string _string = boost::any_cast<string>(args.at(i));
					const char* string = _string.c_str();
					cell* store;
					amx_PushString(amx, &store, string, 1, 0);

					if (!str)
						str = store;

					break;
				}

				default:
					throw VaultException("PAWN call: Unknown argument identifier %02X", argl[i]);
			}
		}

		err = amx_Exec(amx, &ret, idx);

		if (err != AMX_ERR_NONE)
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err));

		if (str)
			amx_Release(amx, str);
	}

	catch (...)
	{
		if (str)
			amx_Release(amx, str);

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
