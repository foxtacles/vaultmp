#include "PAWN.h"
#include "Script.h"

using namespace std;
using namespace RakNet;

extern "C" {
	int AMXAPI amx_CoreInit(AMX*);
	int AMXAPI amx_ConsoleInit(AMX*);
	int AMXAPI amx_FloatInit(AMX*);
	int AMXAPI amx_TimeInit(AMX*);
	int AMXAPI amx_StringInit(AMX*);
	int AMXAPI amx_FileInit(AMX*);
}

static vector<const char*> strings;
static vector<pair<cell*, double>> floats;

void free_strings() {
	for (const auto* value : strings)
		delete[] value;

	strings.clear();
}

void free_floats() {
	for (const auto& value : floats)
		*value.first = amx_ftoc(value.second);

	floats.clear();
}

void after_call() {
	free_strings();
	free_floats();
}

template<typename R>
using FunctionPointerEllipsis = R(*)(...);

template<typename R, unsigned int I, unsigned int F>
struct PAWN_extract_ {
	inline static R PAWN_extract(AMX*&&, const cell*&& params) {
		return static_cast<R>(forward<const cell*>(params)[I]);
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_extract_<double, I, F> {
	inline static double PAWN_extract(AMX*&&, const cell*&& params) {
		return amx_ctof(forward<const cell*>(params)[I]);
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_extract_<const char*, I, F> {
	inline static const char* PAWN_extract(AMX*&& amx, const cell*&& params) {
		int len;
		cell* source;

		source = amx_Address(amx, params[I]);
		amx_StrLen(source, &len);

		char* value = new char[len + 1];
		amx_GetString(value, source, 0, UNLIMITED);
		strings.emplace_back(value);

		return value;
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_extract_<double*, I, F> {
	inline static double* PAWN_extract(AMX*&& amx, const cell*&& params) {
		cell* dest;

		dest = amx_Address(amx, params[I]);
		floats.emplace_back(dest, 0.00);

		return &floats.back().second;
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_dispatch_ {
	template<typename R, typename... Args>
	inline static R PAWN_dispatch(AMX*&& amx, const cell*&& params, Args&&... args) {
		constexpr ScriptFunctionData const& F_ = Script::functions[F];
		return PAWN_dispatch_<I - 1, F>::template PAWN_dispatch<R>(forward<AMX*>(amx), forward<const cell*>(params), PAWN_extract_<typename CharType<F_.func.types[I - 1]>::type, I, F>::PAWN_extract(forward<AMX*>(amx), forward<const cell*>(params)), forward<Args>(args)...);
	}
};

template<unsigned int F>
struct PAWN_dispatch_<0, F> {
	template<typename R, typename... Args>
	inline static R PAWN_dispatch(AMX*&&, const cell*&&, Args&&... args) {
		constexpr ScriptFunctionData const& F_ = Script::functions[F];
		return reinterpret_cast<FunctionPointerEllipsis<R>>(F_.func.addr)(forward<Args>(args)...);
	}
};

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret == 'v', cell>::type wrapper(AMX* amx, const cell* params) {
	PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<void>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call();
	return 1;
}

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret == 'f', cell>::type wrapper(AMX* amx, const cell* params) {
	double value = PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<double>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call();
	return amx_ftoc(value);
}

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret == 's', cell>::type wrapper(AMX* amx, const cell* params) {
	const char* value = PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<const char*>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call();

	if (*value)
	{
		cell* dest = amx_Address(amx, params[Script::functions[I].func.numargs + 1]);
		amx_SetString(dest, value, 1, 0, strlen(value) + 1);
		return 1;
	}

	return 0;
}

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret != 'v' && Script::functions[I].func.ret != 'f' && Script::functions[I].func.ret != 's', cell>::type wrapper(AMX* amx, const cell* params) {
	auto result = PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<typename CharType<Script::functions[I].func.ret>::type>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call();
	return result;
}

AMX_NATIVE_INFO PAWN::functions[] =
{
	{"timestamp",  wrapper<0>},
	{"CreateTimer", PAWN::CreateTimer},
	{"CreateTimerEx", PAWN::CreateTimerEx},
	{"KillTimer", wrapper<3>},
	{"MakePublic", PAWN::MakePublic},
	{"CallPublic", PAWN::CallPublic},

	{"SetServerName", wrapper<6>},
	{"SetServerMap", wrapper<7>},
	{"SetServerRule", wrapper<8>},
	{"GetGameCode", wrapper<9>},
	{"GetMaximumPlayers", wrapper<10>},
	{"GetCurrentPlayers", wrapper<11>},

	{"ValueToString", wrapper<12>},
	{"AxisToString", wrapper<13>},
	{"AnimToString", wrapper<14>},
	{"BaseToString", wrapper<15>},

	{"UIMessage", wrapper<16>},
	{"ChatMessage", wrapper<17>},
	{"SetRespawn", wrapper<18>},
	{"SetSpawnCell", wrapper<19>},
	{"SetGameWeather", wrapper<20>},
	{"SetGameTime", wrapper<21>},
	{"SetGameYear", wrapper<22>},
	{"SetGameMonth", wrapper<23>},
	{"SetGameDay", wrapper<24>},
	{"SetGameHour", wrapper<25>},
	{"SetTimeScale", wrapper<26>},
	{"IsValid", wrapper<27>},
	{"IsObject", wrapper<28>},
	{"IsItem", wrapper<29>},
	{"IsContainer", wrapper<30>},
	{"IsActor", wrapper<31>},
	{"IsPlayer", wrapper<32>},
	{"IsCell", wrapper<33>},
	{"IsInterior", wrapper<34>},
	{"GetType", wrapper<35>},
	{"GetConnection", wrapper<36>},
	{"GetCount", wrapper<37>},
	{"GetList", PAWN::GetList},
	{"GetGameWeather", wrapper<39>},
	{"GetGameTime", wrapper<40>},
	{"GetGameYear", wrapper<41>},
	{"GetGameMonth", wrapper<42>},
	{"GetGameDay", wrapper<43>},
	{"GetGameHour", wrapper<44>},
	{"GetTimeScale", wrapper<45>},

	{"GetID", wrapper<46>},
	{"GetReference", wrapper<47>},
	{"GetBase", wrapper<48>},
	{"GetPos", wrapper<49>},
	{"GetAngle", wrapper<50>},
	{"GetCell", wrapper<51>},
	{"GetLock", wrapper<52>},
	{"GetOwner", wrapper<53>},
	{"GetBaseName", wrapper<54>},
	{"IsNearPoint", wrapper<55>},
	{"GetItemContainer", wrapper<56>},
	{"GetItemCount", wrapper<57>},
	{"GetItemCondition", wrapper<58>},
	{"GetItemEquipped", wrapper<59>},
	{"GetItemSilent", wrapper<60>},
	{"GetItemStick", wrapper<61>},
	{"GetContainerItemCount", wrapper<62>},
	{"GetContainerItemList", PAWN::GetContainerItemList},
	{"GetActorValue", wrapper<64>},
	{"GetActorBaseValue", wrapper<65>},
	{"GetActorIdleAnimation", wrapper<66>},
	{"GetActorMovingAnimation", wrapper<67>},
	{"GetActorWeaponAnimation", wrapper<68>},
	{"GetActorAlerted", wrapper<69>},
	{"GetActorSneaking", wrapper<70>},
	{"GetActorDead", wrapper<71>},
	{"GetActorBaseRace", wrapper<72>},
	{"GetActorBaseSex", wrapper<73>},
	{"IsActorJumping", wrapper<74>},
	{"GetPlayerRespawn", wrapper<75>},
	{"GetPlayerSpawnCell", wrapper<76>},

	{"CreateObject", wrapper<77>},
	{"DestroyObject", wrapper<78>},
	{"SetPos", wrapper<79>},
	{"SetAngle", wrapper<80>},
	{"SetCell", wrapper<81>},
	{"SetLock", wrapper<82>},
	{"SetOwner", wrapper<83>},
	{"SetBaseName", wrapper<84>},
	{"CreateItem", wrapper<85>},
	{"SetItemCount", wrapper<86>},
	{"SetItemCondition", wrapper<87>},
	{"CreateContainer", wrapper<88>},
	{"AddItem", wrapper<89>},
	{"RemoveItem", wrapper<90>},
	{"RemoveAllItems", wrapper<91>},
	{"CreateActor", wrapper<92>},
	{"SetActorValue", wrapper<93>},
	{"SetActorBaseValue", wrapper<94>},
	{"EquipItem", wrapper<95>},
	{"UnequipItem", wrapper<96>},
	{"PlayIdle", wrapper<97>},
	{"SetActorMovingAnimation", wrapper<98>},
	{"SetActorWeaponAnimation",wrapper<99>},
	{"SetActorAlerted", wrapper<100>},
	{"SetActorSneaking", wrapper<101>},
	{"FireWeapon", wrapper<102>},
	{"KillActor", wrapper<103>},
	{"SetActorBaseRace", wrapper<104>},
	{"AgeActorBaseRace", wrapper<105>},
	{"SetActorBaseSex", wrapper<106>},
	{"SetPlayerRespawn", wrapper<107>},
	{"SetPlayerSpawnCell", wrapper<108>},

	{0, 0}
};

int PAWN::RegisterVaultmpFunctions(AMX* amx)
{
	return PAWN::Register(amx, PAWN::functions, -1);
}

cell PAWN::CreateTimer(AMX* amx, const cell* params)
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

cell PAWN::CreateTimerEx(AMX* amx, const cell* params)
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
		throw VaultException("Script call: Number of arguments does not match definition").stacktrace();

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
				throw VaultException("PAWN call: Unknown argument identifier %02X", def[i]).stacktrace();
		}
	}

	return Script::CreateTimerPAWNEx(&name[0], amx, params[2], &def[0], args);
}

cell PAWN::MakePublic(AMX* amx, const cell* params)
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

cell PAWN::CallPublic(AMX* amx, const cell* params)
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
		throw VaultException("Script call: Number of arguments does not match definition").stacktrace();

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
				throw VaultException("PAWN call: Unknown argument identifier %02X", def[i]).stacktrace();
		}
	}

	return Script::CallPublicPAWN(&name[0], args);
}

cell PAWN::GetList(AMX* amx, const cell* params)
{
	NetworkID* data;
	cell* dest = amx_Address(amx, params[2]);
	unsigned int size = Script::GetList(params[1], &data);

	for (unsigned int i = 0; i < size; ++i)
		dest[i] = data[i];

	return size;
}

cell PAWN::GetContainerItemList(AMX* amx, const cell* params)
{
	NetworkID* data;
	cell* dest = amx_Address(amx, params[2]);
	unsigned int size = Script::GetContainerItemList(params[1], &data);

	for (unsigned int i = 0; i < size; ++i)
		dest[i] = data[i];

	return size;
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
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err)).stacktrace();

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
					throw VaultException("PAWN call: Unknown argument identifier %02X", argl[i]).stacktrace();
			}
		}

		err = amx_Exec(amx, &ret, idx);

		if (err != AMX_ERR_NONE)
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err)).stacktrace();

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
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err)).stacktrace();

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
					throw VaultException("PAWN call: Unknown argument identifier %02X", argl[i]).stacktrace();
			}
		}

		err = amx_Exec(amx, &ret, idx);

		if (err != AMX_ERR_NONE)
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err)).stacktrace();

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
