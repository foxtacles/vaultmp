#include "PAWN.h"
#include "Script.h"
#include "Public.h"
#include "amx/amx.h"
#include "amx/amxaux.h"

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
static pair<cell*, NetworkID*> data = {nullptr, nullptr};

void free_strings() noexcept {
	for (const auto* value : strings)
		delete[] value;

	strings.clear();
}

void free_floats() noexcept {
	for (const auto& value : floats)
		*value.first = amx_ftoc(value.second);

	floats.clear();
}

void free_data(unsigned int size) noexcept {
	if (data.first && data.second)
		for (unsigned int i = 0; i < size; ++i)
			data.first[i] = data.second[i];

	data.first = nullptr;
	data.second = nullptr;
}

void after_call() noexcept {
	free_strings();
	free_floats();
}

template<typename R>
void after_call(const R&) noexcept {
	free_strings();
	free_floats();
}

template<>
void after_call(const unsigned int& result) noexcept {
	free_strings();
	free_floats();
	free_data(result);
}

template<typename R, unsigned int I, unsigned int F>
struct PAWN_extract_ {
	inline static R PAWN_extract(AMX*&&, const cell*&& params) noexcept {
		return static_cast<R>(forward<const cell*>(params)[I]);
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_extract_<double, I, F> {
	inline static double PAWN_extract(AMX*&&, const cell*&& params) noexcept {
		return amx_ctof(forward<const cell*>(params)[I]);
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_extract_<const char*, I, F> {
	inline static const char* PAWN_extract(AMX*&& amx, const cell*&& params) noexcept {
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
	inline static double* PAWN_extract(AMX*&& amx, const cell*&& params) noexcept {
		floats.emplace_back(amx_Address(amx, params[I]), 0.00);
		return &floats.back().second;
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_extract_<NetworkID**, I, F> {
	inline static NetworkID** PAWN_extract(AMX*&& amx, const cell*&& params) noexcept {
		constexpr ScriptFunctionData const& F_ = Script::functions[F];
		static_assert(F_.func.numargs == I, "NetworkID** must be the last parameter");
		data.first = amx_Address(amx, params[I]);
		return &data.second;
	}
};

template<unsigned int I, unsigned int F>
struct PAWN_dispatch_ {
	template<typename R, typename... Args>
	inline static R PAWN_dispatch(AMX*&& amx, const cell*&& params, Args&&... args) noexcept {
		constexpr ScriptFunctionData const& F_ = Script::functions[F];
		return PAWN_dispatch_<I - 1, F>::template PAWN_dispatch<R>(forward<AMX*>(amx), forward<const cell*>(params), PAWN_extract_<typename CharType<F_.func.types[I - 1]>::type, I, F>::PAWN_extract(forward<AMX*>(amx), forward<const cell*>(params)), forward<Args>(args)...);
	}
};

template<unsigned int F>
struct PAWN_dispatch_<0, F> {
	template<typename R, typename... Args>
	inline static R PAWN_dispatch(AMX*&&, const cell*&&, Args&&... args) noexcept {
		constexpr ScriptFunctionData const& F_ = Script::functions[F];
		return reinterpret_cast<FunctionEllipsis<R>>(F_.func.addr)(forward<Args>(args)...);
	}
};

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret == 'v', cell>::type wrapper(AMX* amx, const cell* params) noexcept {
	PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<void>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call();
	return 1;
}

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret == 'f', cell>::type wrapper(AMX* amx, const cell* params) noexcept {
	double value = PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<double>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call();
	return amx_ftoc(value);
}

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret == 's', cell>::type wrapper(AMX* amx, const cell* params) noexcept {
	const char* value = PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<const char*>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call();

	if (value) {
		cell* dest = amx_Address(amx, params[Script::functions[I].func.numargs + 1]);
		amx_SetString(dest, value, 1, 0, strlen(value) + 1);
		return 1;
	}

	return 0;
}

template<unsigned int I>
static typename enable_if<Script::functions[I].func.ret != 'v' && Script::functions[I].func.ret != 'f' && Script::functions[I].func.ret != 's', cell>::type wrapper(AMX* amx, const cell* params) noexcept {
	auto result = PAWN_dispatch_<Script::functions[I].func.numargs, I>::template PAWN_dispatch<typename CharType<Script::functions[I].func.ret>::type>(forward<AMX*>(amx), forward<const cell*>(params));
	after_call(result);
	return result;
}

template<unsigned int I>
struct F_ {
	static constexpr AMX_NATIVE_INFO F{Script::functions[I].name, wrapper<I>};
};

AMX_NATIVE_INFO PAWN::functions[] {
	F_<0>::F,
	{"CreateTimer", PAWN::CreateTimer},
	{"CreateTimerEx", PAWN::CreateTimerEx},
	F_<3>::F,
	{"MakePublic", PAWN::MakePublic},
	{"CallPublic", PAWN::CallPublic},
	F_<6>::F,
	F_<7>::F,
	F_<8>::F,
	F_<9>::F,
	F_<10>::F,
	F_<11>::F,
	F_<12>::F,
	F_<13>::F,
	F_<14>::F,
	F_<15>::F,
	F_<16>::F,
	F_<17>::F,
	F_<18>::F,
	F_<19>::F,
	F_<20>::F,
	F_<21>::F,
	F_<22>::F,
	F_<23>::F,
	F_<24>::F,
	F_<25>::F,
	F_<26>::F,
	F_<27>::F,
	F_<28>::F,
	F_<29>::F,
	F_<30>::F,
	F_<31>::F,
	F_<32>::F,
	F_<33>::F,
	F_<34>::F,
	F_<35>::F,
	F_<36>::F,
	F_<37>::F,
	F_<38>::F,
	F_<39>::F,
	F_<40>::F,
	F_<41>::F,
	F_<42>::F,
	F_<43>::F,
	F_<44>::F,
	F_<45>::F,
	F_<46>::F,
	F_<47>::F,
	F_<48>::F,
	F_<49>::F,
	F_<50>::F,
	F_<51>::F,
	F_<52>::F,
	F_<53>::F,
	F_<54>::F,
	F_<55>::F,
	F_<56>::F,
	F_<57>::F,
	F_<58>::F,
	F_<59>::F,
	F_<60>::F,
	F_<61>::F,
	F_<62>::F,
	F_<63>::F,
	F_<64>::F,
	F_<65>::F,
	F_<66>::F,
	F_<67>::F,
	F_<68>::F,
	F_<69>::F,
	F_<70>::F,
	F_<71>::F,
	F_<72>::F,
	F_<73>::F,
	F_<74>::F,
	F_<75>::F,
	F_<76>::F,
	F_<77>::F,
	F_<78>::F,
	F_<79>::F,
	F_<80>::F,
	F_<81>::F,
	F_<82>::F,
	F_<83>::F,
	F_<84>::F,
	F_<85>::F,
	F_<86>::F,
	F_<87>::F,
	F_<88>::F,
	F_<89>::F,
	F_<90>::F,
	F_<91>::F,
	F_<92>::F,
	F_<93>::F,
	F_<94>::F,
	F_<95>::F,
	F_<96>::F,
	F_<97>::F,
	F_<98>::F,
	F_<99>::F,
	F_<100>::F,
	F_<101>::F,
	F_<102>::F,
	F_<103>::F,
	F_<104>::F,
	F_<105>::F,
	F_<106>::F,
	F_<107>::F,
	F_<108>::F,
	F_<109>::F,
	F_<110>::F,
	F_<111>::F,
	F_<112>::F,
	F_<113>::F,
	F_<114>::F,
	F_<115>::F,
	F_<116>::F,
	F_<117>::F,
	F_<118>::F,
	F_<119>::F,
	F_<120>::F,
	F_<121>::F,
	F_<122>::F,
	F_<123>::F,
	F_<124>::F,
	F_<125>::F,
	F_<126>::F,
	F_<127>::F,
	F_<128>::F,
	F_<129>::F,
	F_<130>::F,
	F_<131>::F,
	F_<132>::F,
	F_<133>::F,
	F_<134>::F,
	F_<135>::F,
	F_<136>::F,
	F_<137>::F,
	F_<138>::F,
	F_<139>::F,
	F_<140>::F,
	F_<141>::F,
	F_<142>::F,
	F_<143>::F,
	F_<144>::F,
	F_<145>::F,
	F_<146>::F,
	F_<147>::F,
	F_<148>::F,
	F_<149>::F,
	F_<150>::F,
	F_<151>::F,
	F_<152>::F,
	F_<153>::F,
	F_<154>::F,
	F_<155>::F,
	F_<156>::F,
	F_<157>::F,
	F_<158>::F,
	F_<159>::F,
	F_<160>::F,
	F_<161>::F,
	F_<162>::F,
	F_<163>::F,
	F_<164>::F,
	F_<165>::F,
	F_<166>::F,

	{0, 0}
};

cell PAWN::CreateTimer(AMX* amx, const cell* params) noexcept
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

cell PAWN::CreateTimerEx(AMX* amx, const cell* params) noexcept
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

	if (count != static_cast<unsigned int>(len))
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

			case 'q':
			{
				args.emplace_back((signed int) *data);
				break;
			}

			case 'l':
			{
				args.emplace_back((unsigned long long) *data);
				break;
			}

			case 'w':
			{
				args.emplace_back((signed long long) *data);
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

cell PAWN::MakePublic(AMX* amx, const cell* params) noexcept
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

cell PAWN::CallPublic(AMX* amx, const cell* params) noexcept
{
	int len;
	cell* source;

	source = amx_Address(amx, params[1]);
	amx_StrLen(source, &len);
	vector<char> name;
	name.reserve(len + 1);

	amx_GetString(&name[0], source, 0, UNLIMITED);

	string def;

	try
	{
		def = Public::GetDefinition(&name[0]);
	}
	catch (...) { return 0; }

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

			case 'q':
			{
				args.emplace_back((signed int) *data);
				break;
			}

			case 'l':
			{
				args.emplace_back((unsigned long long) *data);
				break;
			}

			case 'w':
			{
				args.emplace_back((signed long long) *data);
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

int PAWN::LoadProgram(AMX* amx, const char* filename, void* memblock)
{
	return aux_LoadProgram(amx, filename, memblock);
}

int PAWN::Init(AMX* amx)
{
	amx_CoreInit(amx);
	amx_ConsoleInit(amx);
	amx_FloatInit(amx);
	amx_TimeInit(amx);
	amx_StringInit(amx);
	amx_FileInit(amx);

	static_assert(sizeof(PAWN::functions) / sizeof(PAWN::functions[0]) - 1 == sizeof(Script::functions) / sizeof(Script::functions[0]), "Not all functions have been mapped to PAWN");

	return amx_Register(amx, PAWN::functions, -1);
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
	int idx;
	return (amx_FindPublic(amx, name, &idx) == AMX_ERR_NONE);
}

cell PAWN::Call(AMX* amx, const char* name, const char* argl, int buf, ...)
{
	va_list args;
	va_start(args, buf);
	cell ret = 0;
	vector<pair<cell*, char*>> strings;

	try
	{
		int idx = 0;
		int err = 0;

		err = amx_FindPublic(amx, name, &idx);

		if (err != AMX_ERR_NONE)
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err)).stacktrace();

		unsigned int len = strlen(argl);
		vector<cell> args_amx;

		for (unsigned int i = 0; i < len; ++i)
		{
			switch (argl[i])
			{
				case 'i':
					args_amx.emplace_back(va_arg(args, unsigned int));
					break;

				case 'q':
					args_amx.emplace_back(va_arg(args, signed int));
					break;

				case 'l':
					args_amx.emplace_back(va_arg(args, unsigned long long));
					break;

				case 'w':
					args_amx.emplace_back(va_arg(args, signed long long));
					break;

				case 'f':
				{
					double value = va_arg(args, double);
					args_amx.emplace_back(amx_ftoc(value));
					break;
				}

				case 'p':
					args_amx.emplace_back(reinterpret_cast<unsigned int>(va_arg(args, void*)));
					break;

				case 's':
					args_amx.emplace_back(reinterpret_cast<unsigned int>(va_arg(args, char*)));
					break;

				default:
					throw VaultException("PAWN call: Unknown argument identifier %02X", argl[i]).stacktrace();
			}
		}

		for (unsigned int i = len; i; --i)
		{
			switch (argl[i - 1])
			{
				case 's':
				{
					char* string = reinterpret_cast<char*>(static_cast<unsigned int>(args_amx[i - 1]));
					cell* store;
					amx_PushString(amx, &store, string, 1, 0);
					strings.emplace_back(store, string);
					break;
				}

				default:
					amx_Push(amx, args_amx[i - 1]);
					break;
			}
		}

		err = amx_Exec(amx, &ret, idx);

		if (err != AMX_ERR_NONE)
			throw VaultException("PAWN runtime error (%d): \"%s\"", err, aux_StrError(err)).stacktrace();

		if (buf != 0)
			for (const auto& str : strings)
				amx_GetString(str.second, str.first, 0, strlen(str.second) + 1);

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

				case 'q':
				{
					cell value = (cell) boost::any_cast<signed int>(args.at(i));
					amx_Push(amx, value);
					break;
				}

				case 'l':
				{
					cell value = (cell) boost::any_cast<unsigned long long>(args.at(i));
					amx_Push(amx, value);
					break;
				}

				case 'w':
				{
					cell value = (cell) boost::any_cast<signed long long>(args.at(i));
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
					string string_ = boost::any_cast<string>(args.at(i));
					cell* store;
					amx_PushString(amx, &store, string_.c_str(), 1, 0);

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
