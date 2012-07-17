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

#include "Script.h"

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

	{"UIMessage", PAWN::vaultmp_UIMessage},
	{"ChatMessage", PAWN::vaultmp_ChatMessage},
	{"SetRespawn", PAWN::vaultmp_SetRespawn},
	{"IsValid", PAWN::vaultmp_IsValid},
	{"IsObject", PAWN::vaultmp_IsObject},
	{"IsItem", PAWN::vaultmp_IsItem},
	{"IsContainer", PAWN::vaultmp_IsContainer},
	{"IsActor", PAWN::vaultmp_IsActor},
	{"IsPlayer", PAWN::vaultmp_IsPlayer},
	{"GetType", PAWN::vaultmp_GetType},
	{"GetCount", PAWN::vaultmp_GetCount},
	{"GetList", PAWN::vaultmp_GetList},

	{"GetReference", PAWN::vaultmp_GetReference},
	{"GetBase", PAWN::vaultmp_GetBase},
	{"GetName", PAWN::vaultmp_GetName},
	{"GetPos", PAWN::vaultmp_GetPos},
	{"GetAngle", PAWN::vaultmp_GetAngle},
	{"GetCell", PAWN::vaultmp_GetCell},
	{"IsNearPoint", PAWN::vaultmp_IsNearPoint},
	{"GetContainerItemCount", PAWN::vaultmp_GetContainerItemCount},
	{"GetActorValue", PAWN::vaultmp_GetActorValue},
	{"GetActorBaseValue", PAWN::vaultmp_GetActorBaseValue},
	{"GetActorMovingAnimation", PAWN::vaultmp_GetActorMovingAnimation},
	{"GetActorAlerted", PAWN::vaultmp_GetActorAlerted},
	{"GetActorSneaking", PAWN::vaultmp_GetActorSneaking},
	{"GetActorDead", PAWN::vaultmp_GetActorDead},
	{"IsActorJumping", PAWN::vaultmp_IsActorJumping},

	{"AddItem", PAWN::vaultmp_AddItem},
	{"RemoveItem", PAWN::vaultmp_RemoveItem},
	{"RemoveAllItems", PAWN::vaultmp_RemoveAllItems},
	{"SetActorValue", PAWN::vaultmp_SetActorValue},
	{"SetActorBaseValue", PAWN::vaultmp_SetActorBaseValue},
	{"EquipItem", PAWN::vaultmp_EquipItem},
	{"UnequipItem", PAWN::vaultmp_UnequipItem},
	{"KillActor", PAWN::vaultmp_KillActor},
	{"SetPlayerRespawn", PAWN::vaultmp_SetPlayerRespawn},

	{0, 0}
};

int PAWN::RegisterVaultmpFunctions(AMX* amx)
{
	return PAWN::Register(amx, PAWN::vaultmp_functions, -1);
}

cell PAWN::vaultmp_timestamp(AMX* amx, const cell* params)
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
	char name[len + 1];

	amx_GetString(name, source, 0, UNLIMITED);

	return Script::CreateTimerPAWN(name, amx, params[2]);
}

cell PAWN::vaultmp_CreateTimerEx(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	char name[len + 1];

	amx_GetString(name, source, 0, UNLIMITED);

	source = amx_Address(amx, params[3]);
	amx_StrLen(source, &len);
	char def[len + 1];

	amx_GetString(def, source, 0, UNLIMITED);

	vector<boost::any> args;
	unsigned int count = (params[0] / sizeof(cell)) - 3;

	if (count != len)
		throw VaultException("Script call: Number of arguments does not match definition");

	for (int i = 0; i < count; ++i)
	{
		cell* data = amx_Address(amx, params[i + 4]);

		switch (def[i])
		{
			case 'i':
			{
				args.push_back((unsigned int) *data);
				break;
			}

			case 'l':
			{
				args.push_back((unsigned long long) *data);
				break;
			}

			case 'f':
			{
				args.push_back((double) amx_ctof(*data));
				break;
			}

			case 's':
			{
				amx_StrLen(data, &len);
				char str[len + 1];
				amx_GetString(str, data, 0, UNLIMITED);
				args.push_back(string(str));
				break;
			}

			default:
				throw VaultException("PAWN call: Unknown argument identifier %02X", def[i]);
		}
	}

	return Script::CreateTimerPAWNEx(name, amx, params[2], def, args);
}

cell PAWN::vaultmp_KillTimer(AMX* amx, const cell* params)
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
	char real[len + 1];

	amx_GetString(real, source, 0, UNLIMITED);

	source = amx_Address(amx, params[2]);
	amx_StrLen(source, &len);
	char name[len + 1];

	amx_GetString(name, source, 0, UNLIMITED);

	source = amx_Address(amx, params[3]);
	amx_StrLen(source, &len);
	char def[len + 1];

	amx_GetString(def, source, 0, UNLIMITED);

	Script::MakePublicPAWN(real, amx, name, def);

	return 1;
}

cell PAWN::vaultmp_CallPublic(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	char name[len + 1];

	amx_GetString(name, source, 0, UNLIMITED);

	string def = Public::GetDefinition(string(name));

	vector<boost::any> args;
	unsigned int count = (params[0] / sizeof(cell)) - 1;

	if (count != def.length())
		throw VaultException("Script call: Number of arguments does not match definition");

	for (int i = 0; i < count; ++i)
	{
		cell* data = amx_Address(amx, params[i + 2]);

		switch (def[i])
		{
			case 'i':
			{
				args.push_back((unsigned int) *data);
				break;
			}

			case 'l':
			{
				args.push_back((unsigned long long) *data);
				break;
			}

			case 'f':
			{
				args.push_back((double) amx_ctof(*data));
				break;
			}

			case 'p':
			{
				args.push_back((void*) data);
				break;
			}

			case 's':
			{
				amx_StrLen(data, &len);
				char str[len + 1];
				amx_GetString(str, data, 0, UNLIMITED);
				args.push_back(string(str));
				break;
			}

			default:
				throw VaultException("PAWN call: Unknown argument identifier %02X", def[i]);
		}
	}

	return Script::CallPublicPAWN(name, args);
}

cell PAWN::vaultmp_SetServerName(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	char name[len + 1];

	amx_GetString(name, source, 0, UNLIMITED);

	Dedicated::SetServerName(name);

	return 1;
}

cell PAWN::vaultmp_SetServerMap(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	char map[len + 1];

	amx_GetString(map, source, 0, UNLIMITED);

	Dedicated::SetServerMap(map);

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

	char rule[len + 1];
	char value[len2 + 1];

	amx_GetString(rule, source, 0, UNLIMITED);
	amx_GetString(value, source2, 0, UNLIMITED);

	Dedicated::SetServerRule(rule, value);

	return 1;
}

cell PAWN::vaultmp_GetGameCode(AMX* amx, const cell* params)
{
	return Dedicated::GetGameCode();
}

cell PAWN::vaultmp_GetMaximumPlayers(AMX* amx, const cell* params)
{
	return Dedicated::GetMaximumPlayers();
}

cell PAWN::vaultmp_GetCurrentPlayers(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_UIMessage(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[2]);
	amx_StrLen(source, &len);
	char message[len + 1];

	amx_GetString(message, source, 0, UNLIMITED);

	return Script::UIMessage(params[1], message);
}

cell PAWN::vaultmp_ChatMessage(AMX* amx, const cell* params)
{
	int len;
	cell* source;

	source = amx_Address(amx, params[2]);
	amx_StrLen(source, &len);
	char message[len + 1];

	amx_GetString(message, source, 0, UNLIMITED);

	return Script::ChatMessage(params[1], message);
}

cell PAWN::vaultmp_SetRespawn(AMX* amx, const cell* params)
{
	Script::SetRespawn(params[1]);
	return 1;
}

cell PAWN::vaultmp_IsValid(AMX* amx, const cell* params)
{
	return Script::IsValid(params[1]);
}

cell PAWN::vaultmp_IsObject(AMX* amx, const cell* params)
{
	return Script::IsObject(params[1]);
}

cell PAWN::vaultmp_IsItem(AMX* amx, const cell* params)
{
	return Script::IsItem(params[1]);
}

cell PAWN::vaultmp_IsContainer(AMX* amx, const cell* params)
{
	return Script::IsContainer(params[1]);
}

cell PAWN::vaultmp_IsActor(AMX* amx, const cell* params)
{
	return Script::IsActor(params[1]);
}

cell PAWN::vaultmp_IsPlayer(AMX* amx, const cell* params)
{
	return Script::IsPlayer(params[1]);
}

cell PAWN::vaultmp_GetType(AMX* amx, const cell* params)
{
	return GameFactory::GetType((NetworkID) params[1]);
}

cell PAWN::vaultmp_GetCount(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetReference(AMX* amx, const cell* params)
{
	return Script::GetReference(params[1]);
}

cell PAWN::vaultmp_GetBase(AMX* amx, const cell* params)
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

cell PAWN::vaultmp_GetCell(AMX* amx, const cell* params)
{
	return Script::GetCell(params[1]);
}

cell PAWN::vaultmp_IsNearPoint(AMX* amx, const cell* params)
{
	return Script::IsNearPoint(params[1], amx_ctof(params[2]), amx_ctof(params[3]), amx_ctof(params[4]), amx_ctof(params[5]));
}

cell PAWN::vaultmp_GetContainerItemCount(AMX* amx, const cell* params)
{
	return Script::GetContainerItemCount(params[1], params[2]);
}

cell PAWN::vaultmp_GetActorValue(AMX* amx, const cell* params)
{
	double value = Script::GetActorValue(params[1], params[2]);
	return amx_ftoc(value);
}

cell PAWN::vaultmp_GetActorBaseValue(AMX* amx, const cell* params)
{
	double value = Script::GetActorBaseValue(params[1], params[2]);
	return amx_ftoc(value);
}

cell PAWN::vaultmp_GetActorMovingAnimation(AMX* amx, const cell* params)
{
	return Script::GetActorMovingAnimation(params[1]);
}

cell PAWN::vaultmp_GetActorAlerted(AMX* amx, const cell* params)
{
	return Script::GetActorAlerted(params[1]);
}

cell PAWN::vaultmp_GetActorSneaking(AMX* amx, const cell* params)
{
	return Script::GetActorSneaking(params[1]);
}

cell PAWN::vaultmp_GetActorDead(AMX* amx, const cell* params)
{
	return Script::GetActorDead(params[1]);
}

cell PAWN::vaultmp_IsActorJumping(AMX* amx, const cell* params)
{
	return Script::IsActorJumping(params[1]);
}

cell PAWN::vaultmp_AddItem(AMX* amx, const cell* params)
{
	return Script::AddItem(params[1], params[2], params[3], amx_ctof(params[4]), params[5]);
}

cell PAWN::vaultmp_RemoveItem(AMX* amx, const cell* params)
{
	return Script::RemoveItem(params[1], params[2], params[3], params[4]);
}

cell PAWN::vaultmp_RemoveAllItems(AMX* amx, const cell* params)
{
	Script::RemoveAllItems(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetActorValue(AMX* amx, const cell* params)
{
	Script::SetActorValue(params[1], params[2], amx_ctof(params[3]));
	return 1;
}

cell PAWN::vaultmp_SetActorBaseValue(AMX* amx, const cell* params)
{
	Script::SetActorBaseValue(params[1], params[2], amx_ctof(params[3]));
	return 1;
}

cell PAWN::vaultmp_EquipItem(AMX* amx, const cell* params)
{
	return Script::EquipItem(params[1], params[2], params[3], params[4]);
}

cell PAWN::vaultmp_UnequipItem(AMX* amx, const cell* params)
{
	return Script::UnequipItem(params[1], params[2], params[3], params[4]);
}

cell PAWN::vaultmp_KillActor(AMX* amx, const cell* params)
{
	Script::KillActor(params[1]);
	return 1;
}

cell PAWN::vaultmp_SetPlayerRespawn(AMX* amx, const cell* params)
{
	Script::SetPlayerRespawn(params[1], params[2]);
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
					strings.push_back(pair<cell*, char*>(store, string));
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
