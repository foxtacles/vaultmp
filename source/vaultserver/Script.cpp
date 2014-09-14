#include "Script.hpp"
#include "Timer.hpp"
#include "Public.hpp"
#include "Client.hpp"
#include "Network.hpp"
#include "Game.hpp"
#include "amx/amxaux.h"
#include "time/time64.h"

using namespace std;
using namespace RakNet;
using namespace Values;

Script::ScriptList Script::scripts;
Script::DeletedObjects Script::deletedStatic;
Script::GameTime Script::time;
Script::GameWeather Script::weather;

template<typename... Types>
constexpr char TypeString<Types...>::value[];
constexpr ScriptFunctionData Script::functions[];
constexpr ScriptCallbackData Script::callbacks[];

Script::Script(const char* path)
{
	FILE* file = fopen(path, "rb");

	if (!file)
		throw VaultException("Script not found: %s", path).stacktrace();

	fclose(file);

#ifdef __WIN32__
	if (strstr(path, ".dll"))
#else
	if (strstr(path, ".so"))
#endif
	{
		SystemInterface<>::lib_t handle;
#ifdef __WIN32__
		handle = LoadLibrary(path);
#else
		handle = dlopen(path, RTLD_LAZY);
#endif

		if (!handle)
			throw VaultException("Was not able to load C++ script: %s", path).stacktrace();

		try
		{
			this->lib = handle;
			this->cpp_script = true;

			const char* vaultprefix = GetScript<const char*>("vaultprefix");
			string vpf(vaultprefix);

			for (const auto& function : functions)
				if (!SetScript(string(vpf + function.name).c_str(), function.func.addr))
					printf("Script function pointer not found: %s\n", function.name);
		}
		catch (...)
		{
#ifdef __WIN32__
			FreeLibrary(handle);
#else
			dlclose(handle);
#endif
			throw;
		}
	}
	else if (strstr(path, ".amx"))
	{
		AMX* vaultscript = nullptr;

		try
		{
			vaultscript = new AMX();

			this->amx = vaultscript;
			this->cpp_script = false;
			int err = 0;

			err = PAWN::LoadProgram(vaultscript, path, nullptr);

			if (err != AMX_ERR_NONE)
				throw VaultException("PAWN script %s error (%d): \"%s\"", path, err, aux_StrError(err)).stacktrace();

			err = PAWN::Init(vaultscript);

			if (err != AMX_ERR_NONE)
				throw VaultException("PAWN script %s error (%d): \"%s\"", path, err, aux_StrError(err)).stacktrace();

			cell ret;
			err = PAWN::Exec(amx, &ret, AMX_EXEC_MAIN);

			if (err != AMX_ERR_NONE)
				throw VaultException("PAWN script %s error (%d): \"%s\"", path, err, aux_StrError(err)).stacktrace();
		}
		catch (...)
		{
			PAWN::FreeProgram(vaultscript);
			delete vaultscript;
			throw;
		}
	}
	else
		throw VaultException("Script type not recognized: %s", path).stacktrace();
}

Script::~Script()
{
	if (this->cpp_script)
	{
#ifdef __WIN32__
		FreeLibrary(this->lib);
#else
		dlclose(this->lib);
#endif
	}
	else
	{
		PAWN::FreeProgram(this->amx);
		delete this->amx;
	}
}

void Script::LoadScripts(char* scripts, char* base)
{
	char* token = strtok(scripts, ",");

	try
	{
		while (token)
		{
			char path[MAX_PATH];
			snprintf(path, sizeof(path), "%s/%s/%s", base, SCRIPTS_PATH, token);
			Script::scripts.emplace_back(new Script(path));
			token = strtok(nullptr, ",");
		}
	}
	catch (...)
	{
		UnloadScripts();
		throw;
	}
}

void Script::Initialize()
{
	static_assert(sizeof(chrono::system_clock::rep) == sizeof(Time64_T), "Underlying representation of chrono::system_clock should be 64bit integral");

	deletedStatic.clear();

	time.first = chrono::system_clock::now();
	time.second = 1.0;
	CreateTimer(&Timer_GameTime, 1000);

	weather = DEFAULT_WEATHER;

	auto object_init = [](Object* object, const DB::Reference* reference)
	{
		const auto& name = DB::Record::Lookup(reference->GetBase())->GetDescription();
		const auto& pos = reference->GetPos();
		const auto& angle = reference->GetAngle();
		auto cell = reference->GetCell();
		auto lock = reference->GetLock();
		object->SetName(name);
		object->SetNetworkPos(pos);
		object->SetGamePos(pos);
		object->SetAngle(angle);
		object->SetNetworkCell(cell);
		object->SetGameCell(cell);
		object->SetLockLevel(lock);
	};

	const auto& references = DB::Reference::Get();
	const auto& cells = DB::Reference::GetCells();
	unordered_set<unsigned int> refs_cells;
	transform(cells.begin(), cells.end(), inserter(refs_cells, refs_cells.end()), [](const pair<const unsigned int, vector<DB::Reference*>>& cell) { return cell.first; });

	for (const auto& reference : references)
	{
		// FIXME dlc support
		if (reference.first & 0xFF000000)
			continue;

		const auto* data = reference.second;
		unsigned int cell = data->GetCell();

		switch (Utils::hash(data->GetType().c_str(), data->GetType().length() + 1))
		{
			case Utils::hash("CONT"):
				GameFactory::Operate<Container>(GameFactory::Create<Container, FailPolicy::Exception>(data->GetReference(), data->GetBase()), [&object_init, data](Container* container) {
					object_init(container, data);
				});
				break;

			case Utils::hash("TERM"):
				GameFactory::Operate<Object>(GameFactory::Create<Object, FailPolicy::Exception>(data->GetReference(), data->GetBase()), [&object_init, data](Object* object) {
					object_init(object, data);
					object->SetLockLevel(DB::Terminal::Lookup(data->GetBase())->GetLock());
				});
				break;

			case Utils::hash("STAT"):
				if (refs_cells.count(cell))
					GameFactory::Operate<Object>(GameFactory::Create<Object, FailPolicy::Exception>(data->GetReference(), data->GetBase()), [&object_init, data](Object* object) {
						object_init(object, data);
					});
				break;

			default:
				GameFactory::Operate<Object>(GameFactory::Create<Object, FailPolicy::Exception>(data->GetReference(), data->GetBase()), [&object_init, data](Object* object) {
					object_init(object, data);
				});
		}

		refs_cells.erase(cell);
	}
}

void Script::UnloadScripts()
{
	Timer::TerminateAll();
	Public::DeleteAll();
	scripts.clear();
}

void Script::GetArguments(vector<boost::any>& params, va_list args, const string& def)
{
	params.reserve(def.length());

	try
	{
		for (char c : def)
		{
			switch (c)
			{
				case 'i':
				{
					params.emplace_back(va_arg(args, unsigned int));
					break;
				}

				case 'q':
				{
					params.emplace_back(va_arg(args, signed int));
					break;
				}

				case 'l':
				{
					params.emplace_back(va_arg(args, unsigned long long));
					break;
				}

				case 'w':
				{
					params.emplace_back(va_arg(args, signed long long));
					break;
				}

				case 'f':
				{
					params.emplace_back(va_arg(args, double));
					break;
				}

				case 'p':
				{
					params.emplace_back(va_arg(args, void*));
					break;
				}

				case 's':
				{
					params.emplace_back(string(va_arg(args, const char*)));
					break;
				}

				default:
					throw VaultException("C++ call: Unknown argument identifier %02X", c).stacktrace();
			}
		}
	}

	catch (...)
	{
		va_end(args);
		throw;
	}
}

NetworkID Script::CreateTimer(ScriptFunc timer, unsigned int interval) noexcept
{
	return Timer::MakeTimer(timer, string(), vector<boost::any>(), interval)->GetNetworkID();
}

NetworkID Script::CreateTimerEx(ScriptFunc timer, unsigned int interval, const char* def, ...) noexcept
{
	vector<boost::any> params;

	va_list args;
	va_start(args, def);
	GetArguments(params, args, def);
	va_end(args);

	return Timer::MakeTimer(timer, def, params, interval)->GetNetworkID();
}

NetworkID Script::CreateTimerPAWN(ScriptFuncPAWN timer, AMX* amx, unsigned int interval) noexcept
{
	return Timer::MakeTimer(timer, amx, string(), vector<boost::any>(), interval)->GetNetworkID();
}

NetworkID Script::CreateTimerPAWNEx(ScriptFuncPAWN timer, AMX* amx, unsigned int interval, const char* def, const vector<boost::any>& args) noexcept
{
	return Timer::MakeTimer(timer, amx, string(def), args, interval)->GetNetworkID();
}

void Script::SetupObject(Object* object, unsigned int cell, float X, float Y, float Z) noexcept
{
	object->SetNetworkCell(cell);
	object->SetGameCell(cell);
	object->SetNetworkPos(tuple<float, float, float>{X, Y, Z});
	object->SetGamePos(tuple<float, float, float>{X, Y, Z});
}

void Script::SetupItem(Item* item, unsigned int cell, float X, float Y, float Z) noexcept
{
	SetupObject(item, cell, X, Y, Z);

	item->SetItemCount(1);
	item->SetItemCondition(100.0);
}

void Script::SetupContainer(Container* container, unsigned int cell, float X, float Y, float Z) noexcept
{
	SetupObject(container, cell, X, Y, Z);
}

void Script::SetupActor(Actor* actor, unsigned int cell, float X, float Y, float Z) noexcept
{
	SetupContainer(actor, cell, X, Y, Z);

	unsigned int baseID = actor->GetBase();

	if (!DB::Record::Lookup(baseID, "CREA"))
	{
		const DB::NPC* npc = *DB::NPC::Lookup(baseID);
		unsigned int race = npc->GetRace();
		signed int age = DB::Race::Lookup(npc->GetOriginalRace())->GetAgeDifference(race);
		bool female = npc->IsFemale();

		actor->SetActorRace(race);
		actor->SetActorAge(age);
		actor->SetActorFemale(female);
	}
	else
		actor->SetActorRace(UINT_MAX);
}

void Script::KillTimer(NetworkID id) noexcept
{
	if (!id)
		id = Timer::LastTimer();

	Timer::Terminate(id);
}

void Script::MakePublic(ScriptFunc _public, const char* name, const char* def) noexcept
{
	Public::MakePublic(_public, name, def);
}

void Script::MakePublicPAWN(ScriptFuncPAWN _public, AMX* amx, const char* name, const char* def) noexcept
{
	Public::MakePublic(_public, amx, name, def);
}

unsigned long long Script::CallPublic(const char* name, ...) noexcept
{
	vector<boost::any> params;

	try
	{
		string def = Public::GetDefinition(name);

		va_list args;
		va_start(args, name);
		GetArguments(params, args, def);
		va_end(args);

		return Public::Call(name, params);
	}
	catch (...) {}

	return 0;
}

unsigned long long Script::CallPublicPAWN(const char* name, const vector<boost::any>& args) noexcept
{
	try
	{
		return Public::Call(name, args);
	}
	catch (...) {}

	return 0;
}

bool Script::IsPAWN(const char* name) noexcept
{
	try
	{
		return Public::IsPAWN(name);
	}
	catch (...) {}

	return false;
}

unsigned long long Script::Timer_Respawn(NetworkID id) noexcept
{
	if (!GameFactory::Exists<Player>(id))
	{
		KillTimer(); // Player has already left the server
		return 0;
	}

	RakNetGUID guid = Client::GetClientFromPlayer(id)->GetGUID();

	for (const auto& value : Actor::default_values)
	{
		SetActorBaseValue(id, value.first, value.second.first);
		SetActorValue(id, value.first, value.second.second);
	}

	Network::Queue({{
		PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, false, 0, 0),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid}
	});

	KillTimer();

	return 1;
}

unsigned long long Script::Timer_GameTime() noexcept
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	time.first += chrono::milliseconds(static_cast<unsigned long long>(1000ull * time.second));

	t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();
	TM _tm_new;
	gmtime64_r(&t, &_tm_new);

	NetworkResponse response;

	if (_tm.tm_year != _tm_new.tm_year)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, _tm_new.tm_year + 1900),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	if (_tm.tm_mon != _tm_new.tm_mon)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, _tm_new.tm_mon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	if (_tm.tm_mday != _tm_new.tm_mday)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, _tm_new.tm_mday),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	if (_tm.tm_hour != _tm_new.tm_hour)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, _tm_new.tm_hour),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	if (!response.empty())
	{
		Network::Queue(move(response));
		Call<CBI("OnGameTimeChange")>(static_cast<unsigned int>(_tm_new.tm_year + 1900), static_cast<unsigned int>(_tm_new.tm_mon), static_cast<unsigned int>(_tm_new.tm_mday), static_cast<unsigned int>(_tm_new.tm_hour));
	}

	return 1;
}

const char* Script::ValueToString(unsigned char index) noexcept
{
	static string value;
	auto it = find_if(API::values.begin(), API::values.end(), [index](const decltype(API::values)::value_type& value) { return value.second == index; });
	value.assign(it != API::values.end() ? it->first : "");
	return value.c_str();
}

const char* Script::AxisToString(unsigned char index) noexcept
{
	static string axis;
	auto it = find_if(API::axis.begin(), API::axis.end(), [index](const decltype(API::axis)::value_type& axis) { return axis.second == index; });
	axis.assign(it != API::axis.end() ? it->first : "");
	return axis.c_str();
}

const char* Script::AnimToString(unsigned char index) noexcept
{
	static string anim;
	auto it = find_if(API::anims.begin(), API::anims.end(), [index](const decltype(API::anims)::value_type& anims) { return anims.second == index; });
	anim.assign(it != API::anims.end() ? it->first : "");
	return anim.c_str();
}

const char* Script::BaseToString(unsigned int baseID) noexcept
{
	static string base;
	base.clear();

	auto record = DB::Record::Lookup(baseID);

	if (record)
		base.assign(record->GetName());

	return base.c_str();
}

const char* Script::BaseToType(unsigned int baseID) noexcept
{
	static string base;
	base.clear();

	auto record = DB::Record::Lookup(baseID);

	if (record)
		base.assign(record->GetType());

	return base.c_str();
}

bool Script::Kick(NetworkID id) noexcept
{
	return GameFactory::Operate<Player, BOOL_VALIDATED>(id, [id](Player*) {
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_GAME_END>(Reason::ID_REASON_KICK),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()}
		});
	});
}

bool Script::UIMessage(NetworkID id, const char* message, unsigned char emoticon) noexcept
{
	return GameFactory::Operate<Player, BOOL_EXPECTED>(id, [id, message, emoticon](Player* player) {
		if (id && !player)
			throw VaultException("Invalid parameters: player doesn't exist").stacktrace();

		string message_(message);

		if (message_.length() > MAX_MESSAGE_LENGTH)
			message_.resize(MAX_MESSAGE_LENGTH);

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_GAME_MESSAGE>(message_, emoticon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(nullptr)}
		});
	});
}

bool Script::ChatMessage(NetworkID id, const char* message) noexcept
{
	return GameFactory::Operate<Player, BOOL_EXPECTED>(id, [id, message](Player* player) {
		if (id && !player)
			throw VaultException("Invalid parameters: player doesn't exist").stacktrace();

		string message_(message);

		if (message_.length() > MAX_MESSAGE_LENGTH)
			message_.resize(MAX_MESSAGE_LENGTH);

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_GAME_CHAT>(message_),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(nullptr)}
		});
	});
}

void Script::SetSpawnCell(unsigned int cell) noexcept
{
	try
	{
		Player::SetSpawnCell(cell);
	}
	catch (...) {}
}

void Script::SetGameWeather(unsigned int weather) noexcept
{
	if (Script::weather == weather)
		return;

	if (DB::Record::IsValidWeather(weather))
	{
		Script::weather = weather;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_GAME_WEATHER>(weather),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});
	}
}

void Script::SetGameTime(signed long long time) noexcept
{
	Time64_T t_new = time;

	TM _tm_new;
	auto success = gmtime64_r(&t_new, &_tm_new);

	if (!success)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(Script::time.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	NetworkResponse response;

	if (_tm.tm_year != _tm_new.tm_year)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, _tm_new.tm_year + 1900),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	if (_tm.tm_mon != _tm_new.tm_mon)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, _tm_new.tm_mon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	if (_tm.tm_mday != _tm_new.tm_mday)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, _tm_new.tm_mday),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	if (_tm.tm_hour != _tm_new.tm_hour)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, _tm_new.tm_hour),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
		);

	Script::time.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t_new));

	if (!response.empty())
	{
		Network::Queue(move(response));
		Call<CBI("OnGameTimeChange")>(static_cast<unsigned int>(_tm_new.tm_year + 1900), static_cast<unsigned int>(_tm_new.tm_mon), static_cast<unsigned int>(_tm_new.tm_mday), static_cast<unsigned int>(_tm_new.tm_hour));
	}
}

void Script::SetGameYear(unsigned int year) noexcept
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (static_cast<unsigned int>(_tm.tm_year) != year)
	{
		_tm.tm_year = year - 1900;
		t = mktime64(&_tm);

		if (t != -1)
		{
			time.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue({{
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, year),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

			Call<CBI("OnGameTimeChange")>(static_cast<unsigned int>(_tm.tm_year + 1900), static_cast<unsigned int>(_tm.tm_mon), static_cast<unsigned int>(_tm.tm_mday), static_cast<unsigned int>(_tm.tm_hour));
		}
	}
}

void Script::SetGameMonth(unsigned int month) noexcept
{
	if (month > 11)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (static_cast<unsigned int>(_tm.tm_mon) != month)
	{
		_tm.tm_mon = month;
		t = mktime64(&_tm);

		if (t != -1)
		{
			time.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue({{
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, month),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

			Call<CBI("OnGameTimeChange")>(static_cast<unsigned int>(_tm.tm_year + 1900), static_cast<unsigned int>(_tm.tm_mon), static_cast<unsigned int>(_tm.tm_mday), static_cast<unsigned int>(_tm.tm_hour));
		}
	}
}

void Script::SetGameDay(unsigned int day) noexcept
{
	if (!day || day > 31)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (static_cast<unsigned int>(_tm.tm_mday) != day)
	{
		_tm.tm_mday = day;
		t = mktime64(&_tm);

		if (t != -1)
		{
			time.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue({{
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, day),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

			Call<CBI("OnGameTimeChange")>(static_cast<unsigned int>(_tm.tm_year + 1900), static_cast<unsigned int>(_tm.tm_mon), static_cast<unsigned int>(_tm.tm_mday), static_cast<unsigned int>(_tm.tm_hour));
		}
	}
}

void Script::SetGameHour(unsigned int hour) noexcept
{
	if (hour > 23)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (static_cast<unsigned int>(_tm.tm_hour) != hour)
	{
		_tm.tm_hour = hour;
		t = mktime64(&_tm);

		if (t != -1)
		{
			time.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue({{
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, hour),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

			Call<CBI("OnGameTimeChange")>(static_cast<unsigned int>(_tm.tm_year + 1900), static_cast<unsigned int>(_tm.tm_mon), static_cast<unsigned int>(_tm.tm_mday), static_cast<unsigned int>(_tm.tm_hour));
		}
	}
}

void Script::SetTimeScale(double scale) noexcept
{
	time.second = scale;
}

bool Script::IsValid(NetworkID id) noexcept
{
	return GameFactory::GetType(id);
}

bool Script::IsReference(NetworkID id) noexcept
{
	return GameFactory::Exists<Reference>(id);
}

bool Script::IsObject(NetworkID id) noexcept
{
	return GameFactory::Exists<Object>(id);
}

bool Script::IsItem(NetworkID id) noexcept
{
	return GameFactory::Exists<Item>(id);
}

bool Script::IsContainer(NetworkID id) noexcept
{
	return GameFactory::Exists<Container>(id);
}

bool Script::IsActor(NetworkID id) noexcept
{
	return GameFactory::Exists<Actor>(id);
}

bool Script::IsPlayer(NetworkID id) noexcept
{
	return GameFactory::Exists<Player>(id);
}

bool Script::IsInterior(unsigned int cell) noexcept
{
	return DB::Interior::Lookup(cell).operator bool();
}

bool Script::IsItemList(NetworkID id) noexcept
{
	return GameFactory::Exists<ItemList>(id);
}

bool Script::IsWindow(NetworkID id) noexcept
{
	return GameFactory::Exists<Window>(id);
}

bool Script::IsButton(NetworkID id) noexcept
{
	return GameFactory::Exists<Button>(id);
}

bool Script::IsText(NetworkID id) noexcept
{
	return GameFactory::Exists<Text>(id);
}

bool Script::IsEdit(NetworkID id) noexcept
{
	return GameFactory::Exists<Edit>(id);
}

bool Script::IsCheckbox(NetworkID id) noexcept
{
	return GameFactory::Exists<Checkbox>(id);
}

bool Script::IsRadioButton(NetworkID id) noexcept
{
	return GameFactory::Exists<RadioButton>(id);
}

bool Script::IsListItem(NetworkID id) noexcept
{
	return GameFactory::Exists<ListItem>(id);
}

bool Script::IsList(NetworkID id) noexcept
{
	return GameFactory::Exists<List>(id);
}

bool Script::IsChatbox(NetworkID id) noexcept
{
	return GameFactory::Operate<Window, RETURN_VALIDATED>(id, [](Window* window) {
		return !window->GetLabel().compare(Window::GUI_MAIN_LABEL);
	});
}

unsigned int Script::GetConnection(NetworkID id) noexcept
{
	unsigned int value = UINT_MAX;
	Client* client = Client::GetClientFromPlayer(id);

	if (client)
		value = client->GetID();

	return value;
}

unsigned int Script::GetList(unsigned int type, NetworkID** data) noexcept
{
	static vector<NetworkID> _data;
	_data = GameFactory::GetByType(type);
	*data = &_data[0];
	return _data.size();
}

unsigned int Script::GetGameWeather() noexcept
{
	return weather;
}

signed long long Script::GetGameTime() noexcept
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();
	return t;
}

unsigned int Script::GetGameYear() noexcept
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_year + 1900;
}

unsigned int Script::GetGameMonth() noexcept
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_mon;
}

unsigned int Script::GetGameDay() noexcept
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_mday;
}

unsigned int Script::GetGameHour() noexcept
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(time.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_hour;
}

double Script::GetTimeScale() noexcept
{
	return time.second;
}

NetworkID Script::GetID(unsigned int refID) noexcept
{
	return GameFactory::Lookup<Reference>(refID);
}

unsigned int Script::GetReference(NetworkID id) noexcept
{
	return GameFactory::Operate<Reference, RETURN_VALIDATED>(id, [](Reference* reference) {
		return reference->GetReference();
	});
}

unsigned int Script::GetBase(NetworkID id) noexcept
{
	return GameFactory::Operate<Reference, RETURN_VALIDATED>(id, [](Reference* reference) {
		return reference->GetBase();
	});
}

void Script::GetPos(NetworkID id, double* X, double* Y, double* Z) noexcept
{
	*X = 0.00;
	*Y = 0.00;
	*Z = 0.00;

	GameFactory::Operate<Object, RETURN_VALIDATED>(id, [X, Y, Z](Object* object) {
		const auto& pos = object->GetNetworkPos();
		*X = get<0>(pos);
		*Y = get<1>(pos);
		*Z = get<2>(pos);
	});
}

void Script::GetAngle(NetworkID id, double* X, double* Y, double* Z) noexcept
{
	*X = 0.00;
	*Y = 0.00;
	*Z = 0.00;

	GameFactory::Operate<Object, RETURN_VALIDATED>(id, [X, Y, Z](Object* object) {
		const auto& angle = object->GetAngle();
		*X = get<0>(angle);
		*Y = get<1>(angle);
		*Z = get<2>(angle);
	});
}

unsigned int Script::GetCell(NetworkID id) noexcept
{
	return GameFactory::Operate<Object, RETURN_VALIDATED>(id, [](Object* object) {
		return object->GetNetworkCell();
	});
}

unsigned int Script::GetLock(NetworkID id) noexcept
{
	return GameFactory::Operate<Object, RETURN_VALIDATED>(id, [](Object* object) {
		return object->GetLockLevel();
	});
}

unsigned int Script::GetOwner(NetworkID id) noexcept
{
	return GameFactory::Operate<Object, RETURN_VALIDATED>(id, [](Object* object) {
		return object->GetOwner();
	});
}

const char* Script::GetBaseName(NetworkID id) noexcept
{
	static string name;

	return GameFactory::Operate<Object, BOOL_VALIDATED>(id, [](Object* object) {
		name.assign(object->GetName());
	}) ? name.c_str() : "";
}

bool Script::IsNearPoint(NetworkID id, double X, double Y, double Z, double R) noexcept
{
	return GameFactory::Operate<Object, RETURN_VALIDATED>(id, [X, Y, Z, R](Object* object) {
		return object->IsNearPoint(X, Y, Z, R);
	});
}

NetworkID Script::GetItemContainer(NetworkID id) noexcept
{
	return GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) {
		return item->GetItemContainer();
	});
}

unsigned int Script::GetItemCount(NetworkID id) noexcept
{
	return GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) {
		return item->GetItemCount();
	});
}

double Script::GetItemCondition(NetworkID id) noexcept
{
	return GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) {
		return item->GetItemCondition();
	});
}

bool Script::GetItemEquipped(NetworkID id) noexcept
{
	return GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) {
		return item->GetItemEquipped();
	});
}

bool Script::GetItemSilent(NetworkID id) noexcept
{
	return GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) {
		return item->GetItemSilent();
	});
}

bool Script::GetItemStick(NetworkID id) noexcept
{
	return GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) {
		return item->GetItemStick();
	});
}

unsigned int Script::GetContainerItemCount(NetworkID id, unsigned int baseID) noexcept
{
	return GameFactory::Operate<ItemList, RETURN_VALIDATED>(id, [id, baseID](ItemList* itemlist) {
		return itemlist->GetItemCount(baseID);
	});
}

unsigned int Script::GetContainerItemList(NetworkID id, NetworkID** data) noexcept
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<ItemList, RETURN_VALIDATED>(id, [id, data](ItemList* itemlist) {
		_data.assign(itemlist->GetItemList().begin(), itemlist->GetItemList().end());
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	});
}

double Script::GetActorValue(NetworkID id, unsigned char index) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [index](Actor* actor) {
		return actor->GetActorValue(index);
	});
}

double Script::GetActorBaseValue(NetworkID id, unsigned char index) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [index](Actor* actor) {
		return actor->GetActorBaseValue(index);
	});
}

unsigned int Script::GetActorIdleAnimation(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorIdleAnimation();
	});
}

unsigned char Script::GetActorMovingAnimation(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorMovingAnimation();
	});
}

unsigned char Script::GetActorWeaponAnimation(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorWeaponAnimation();
	});
}

bool Script::GetActorAlerted(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorAlerted();
	});
}

bool Script::GetActorSneaking(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorSneaking();
	});
}

bool Script::GetActorDead(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorDead();
	});
}

unsigned int Script::GetActorBaseRace(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorRace();
	});
}

bool Script::GetActorBaseSex(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->GetActorFemale();
	});
}

bool Script::IsActorJumping(NetworkID id) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [](Actor* actor) {
		return actor->IsActorJumping();
	});
}

unsigned int Script::GetPlayerRespawnTime(NetworkID id) noexcept
{
	return GameFactory::Operate<Player, RETURN_VALIDATED>(id, [](Player* player) {
		return player->GetPlayerRespawnTime();
	});
}

unsigned int Script::GetPlayerSpawnCell(NetworkID id) noexcept
{
	return GameFactory::Operate<Player, RETURN_VALIDATED>(id, [](Player* player) {
		return player->GetPlayerSpawnCell();
	});
}

bool Script::GetPlayerConsoleEnabled(NetworkID id) noexcept
{
	return GameFactory::Operate<Player, RETURN_VALIDATED>(id, [](Player* player) {
		return player->GetPlayerConsoleEnabled();
	});
}

unsigned int Script::GetPlayerWindowCount(NetworkID id) noexcept
{
	return GameFactory::Operate<Player, RETURN_VALIDATED>(id, [](Player* player) {
		return player->GetPlayerWindows().size();
	});
}

unsigned int Script::GetPlayerWindowList(NetworkID id, NetworkID** data) noexcept
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<Player, RETURN_VALIDATED>(id, [data](Player* player) {
		const auto& windows = player->GetPlayerWindows();
		_data.assign(windows.begin(), windows.end());
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	});
}

NetworkID Script::GetPlayerChatboxWindow(NetworkID id) noexcept
{
	vector<NetworkID> windows;

	return GameFactory::Operate<Player, BOOL_VALIDATED>(id, [&windows](Player* player) {
		windows = player->GetPlayerWindows();
	}) ? *find_if(windows.begin(), windows.end(), [](NetworkID id) { return IsChatbox(id); }) : 0;
}

NetworkID Script::CreateObject(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept
{
	if (!DB::Record::IsValidCoordinate(cell, X, Y, Z))
		return 0;

	NetworkID result = GameFactory::Operate<Object, RETURN_VALIDATED>(GameFactory::Create<Object, FailPolicy::Return>(baseID), [cell, X, Y, Z](Object* object) {
		SetupObject(object, cell, X, Y, Z);

		Network::Queue({{
			object->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return object->GetNetworkID();
	});

	if (result)
		Call<CBI("OnCreate")>(result);

	return result;
}

bool Script::CreateVolatile(NetworkID id, unsigned int baseID, double aX, double aY, double aZ) noexcept
{
	if (!DB::Record::Lookup(baseID, vector<string>{"EXPL", "PROJ"}))
		return false;

	if (!Object::IsValidAngle(Axis_X, aX) || !Object::IsValidAngle(Axis_Z, aZ))
		return false;

	return GameFactory::Operate<Object, BOOL_VALIDATED>(id, [id, baseID, aX, aY, aZ](Object*) {
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_VOLATILE_NEW>(id, baseID, aX, aY, aZ),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});
	});
}

bool Script::DestroyObject(NetworkID id) noexcept
{
	if (!GameFactory::Operate<Reference, RETURN_FACTORY_VALIDATED>(id, [id](FactoryReference& reference) {
		return !vaultcast_test<Player>(reference);
	})) return false;

	Script::Call<CBI("OnDestroy")>(id);

	return GameFactory::Operate<Reference, BOOL_FACTORY_VALIDATED>(id, [id](FactoryReference& reference) {
		GameFactory::Operate<Object, RETURN_VALIDATED>(id, [](Object* object) {
			if (object->IsPersistent())
				deletedStatic[object->GetNetworkCell()].emplace_back(object->GetReference());
		});

		auto container = GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) -> pair<NetworkID, bool> {
			return {item->GetItemContainer(), item->GetItemSilent()};
		});

		if (!container.first || !GameFactory::Is<ItemList>(container.first))
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id, container.second),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

		if (container.first)
		{
			GameFactory::Free(reference);

			GameFactory::Operate<ItemList>(container.first, [id](ItemList* itemlist) {
				itemlist->RemoveItem(id);
			});

			GameFactory::Destroy(id);
		}
		else
			GameFactory::Destroy(reference);
	});
}

bool Script::Activate(NetworkID id, NetworkID actor) noexcept
{
	if (!GameFactory::Operate<Object, RETURN_FACTORY_EXPECTED>({id, actor}, [actor](ExpectedObjects& objects) {
		return objects[0] && ((!objects[1] && !actor) || vaultcast_test<Actor>(objects[1]));
	})) return false;

	Call<CBI("OnActivate")>(id, actor);

	return true;
}

bool Script::SetPos(NetworkID id, double X, double Y, double Z) noexcept
{
	unsigned int new_cell_;

	bool success = GameFactory::Operate<Object, RETURN_FACTORY_VALIDATED>(id, [id, X, Y, Z, &new_cell_](FactoryObject& object) {
		if (object->IsPersistent())
			return false;

		unsigned int cell = object->GetNetworkCell();
		const DB::Exterior* new_cell = nullptr;

		auto exterior = DB::Exterior::Lookup(cell);

		if (exterior)
		{
			exterior = DB::Exterior::Lookup(exterior->GetWorld(), X, Y);

			if (!exterior)
				return false;

			new_cell = *exterior;
		}
		else
		{
			if (!DB::Interior::Lookup(cell)->IsValidCoordinate(X, Y, Z))
				return false;
		}

		if (!object->SetNetworkPos(tuple<float, float, float>{X, Y, Z}))
			return false;

		object->SetGamePos(tuple<float, float, float>{X, Y, Z});

		NetworkResponse response;

		if (new_cell && object->SetNetworkCell((new_cell_ = new_cell->GetBase())))
		{
			object->SetGameCell(new_cell_);

			auto player = vaultcast<Player>(object);

			if (player)
			{
				response.emplace_back(
					PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_cell->GetWorld(), new_cell->GetX(), new_cell->GetY(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
				);

				response.emplace_back(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, player->GetPlayerCellContext(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
				);
			}

			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, new_cell_, X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
			);
		}
		else
		{
			new_cell_ = 0x00000000;

			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(id, X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
			);
		}

		Network::Queue(move(response));

		return true;
	});

	if (success && new_cell_)
		Call<CBI("OnCellChange")>(id, new_cell_);

	return success;
}

bool Script::SetAngle(NetworkID id, double X, double Y, double Z) noexcept
{
	return GameFactory::Operate<Object, RETURN_VALIDATED>(id, [id, X, Y, Z](Object* object) {
		if (object->IsPersistent())
			return false;

		if (!object->SetAngle(tuple<float, float, float>{X, 0.0f, Z}))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(id, X, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});
}

bool Script::SetCell(NetworkID id, unsigned int cell, double X, double Y, double Z) noexcept
{
	bool success = GameFactory::Operate<Object, RETURN_FACTORY_VALIDATED>(id, [id, &cell, X, Y, Z](FactoryObject& object) {
		if (object->IsPersistent())
			return false;

		bool update_pos = X != 0.00 && Y != 0.00 && Z != 0.00;
		const DB::Interior* new_interior = nullptr;
		const DB::Exterior* new_exterior = nullptr;

		auto exterior = DB::Exterior::Lookup(cell);

		if (!exterior)
		{
			auto interior = DB::Interior::Lookup(cell);

			if (!interior)
				return false;

			if (update_pos && !interior->IsValidCoordinate(X, Y, Z))
				return false;

			new_interior = *interior;
		}
		else if (update_pos)
		{
			exterior = DB::Exterior::Lookup(exterior->GetWorld(), X, Y);

			if (!exterior || (new_exterior = *exterior)->GetBase() != cell)
				return false;
		}
		else
			new_exterior = *exterior;

		NetworkResponse response;

		bool state = false;

		if (object->SetNetworkCell(cell))
		{
			object->SetGameCell(cell);

			auto player = vaultcast<Player>(object);

			if (player)
			{
				if (new_interior)
					response.emplace_back(
						PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(id, DB::Record::Lookup(cell, "CELL")->GetName(), false),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
					);
				else
						response.emplace_back(
						PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_exterior->GetWorld(), new_exterior->GetX(), new_exterior->GetY(), false),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
					);

				response.emplace_back(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, player->GetPlayerCellContext(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
				);
			}

			if (update_pos && object->SetNetworkPos(tuple<float, float, float>{X, Y, Z}))
			{
				object->SetGamePos(tuple<float, float, float>{X, Y, Z});

					response.emplace_back(
					PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell, X, Y, Z),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
				);
			}
			else
				response.emplace_back(
					PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell, 0.0, 0.0, 0.0),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
				);

			state = true;
		}
		else
			cell = 0x00000000;

		if (update_pos && object->SetNetworkPos(tuple<float, float, float>{X, Y, Z}))
		{
			object->SetGamePos(tuple<float, float, float>{X, Y, Z});

			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(id, X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)
			);

			state = true;
		}

		if (!response.empty())
			Network::Queue(move(response));

		return state;
	});

	if (success && cell)
		Call<CBI("OnCellChange")>(id, cell);

	return success;
}

bool Script::SetLock(NetworkID id, NetworkID actor, unsigned int lock) noexcept
{
	bool is_terminal;

	bool success = GameFactory::Operate<Object, RETURN_VALIDATED>(id, [id, &lock, &is_terminal](Object* object) mutable {
		if (object->GetLockLevel() == Lock_Broken)
			return false;

		if (lock != Lock_Unlocked)
		{
			lock = ceil(static_cast<double>(lock) / 25.0) * 25;

			if (lock > Lock_VeryHard)
				lock = Lock_Impossible;

			is_terminal = DB::Terminal::Lookup(object->GetBase()).operator bool();

			if (is_terminal)
			{
				if (lock == Lock_Impossible)
					lock = 5;
				else
					lock /= 25;
			}
		}

		if (!object->SetLockLevel(lock))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_LOCK>(id, lock),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});

	if (success)
	{
		if (is_terminal)
		{
			if (lock == 5)
				lock = Lock_Impossible;
			else
				lock *= 25;
		}

		Call<CBI("OnLockChange")>(id, actor, lock);
	}

	return success;
}

bool Script::SetOwner(NetworkID id, unsigned int owner) noexcept
{
	return GameFactory::Operate<Object, RETURN_VALIDATED>(id, [id, owner](Object* object) {
		if (owner == PLAYER_BASE)
			return false;

		auto npc = DB::NPC::Lookup(owner);

		if (!npc || !object->SetOwner(owner))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_OWNER>(id, owner),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});
}

bool Script::SetBaseName(NetworkID id, const char* name) noexcept
{
	if (!name)
		return false;

	string _name(name);

	if (_name.length() > MAX_PLAYER_NAME)
		return false;

	return GameFactory::Operate<Object, RETURN_FACTORY_VALIDATED>(GameFactory::GetByType(ALL_OBJECTS), [id, &_name](FactoryObjects& objects) {
		auto it = find_if(objects.begin(), objects.end(), [id](const FactoryObject& object) { return object->GetNetworkID() == id; });

		if (it == objects.end())
			return false;

		unsigned int baseID = (*it)->GetBase();

		if (baseID == PLAYER_BASE)
			return false;

		DB::Record::Lookup(baseID)->SetDescription(_name);

		for (const auto& object : objects)
		{
			auto item = vaultcast<Item>(object);

			if (item && item->GetItemContainer())
				continue;

			if (object->GetBase() == baseID)
				if (object->SetName(_name))
					Network::Queue({{
						PacketFactory::Create<pTypes::ID_UPDATE_NAME>(object->GetNetworkID(), _name),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
					});
		}

		return true;
	});
}

bool Script::PlaySound(NetworkID id, unsigned int sound)
{
	return GameFactory::Operate<Object, RETURN_VALIDATED>(id, [id, sound](Object*) {
		if (!DB::Record::IsValidSound(sound))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_SOUND>(id, sound),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});
}

NetworkID Script::CreateItem(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept
{
	if (!DB::Record::IsValidCoordinate(cell, X, Y, Z))
		return 0;

	NetworkID result = GameFactory::Operate<Item, RETURN_VALIDATED>(GameFactory::Create<Item, FailPolicy::Return>(baseID), [cell, X, Y, Z](Item* item) {
		SetupItem(item, cell, X, Y, Z);

		Network::Queue({{
			item->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return item->GetNetworkID();
	});

	if (result)
		Call<CBI("OnCreate")>(result);

	return result;
}

NetworkID Script::SetItemContainer(NetworkID id, NetworkID container) noexcept
{
	unsigned int baseID, count;
	double condition;
	bool equipped, silent, stick;

	if (!GameFactory::Operate<Item, RETURN_VALIDATED>(id, [id, container, &baseID, &count, &condition, &equipped, &silent, &stick](Item* item) {
		if (item->GetItemContainer() == container)
			return false;

		baseID = item->GetBase();
		count = item->GetItemCount();
		condition = item->GetItemCondition();
		equipped = item->GetItemEquipped();
		silent = item->GetItemSilent();
		stick = item->GetItemStick();
		return true;
	})) return 0;

	NetworkID new_id = AddItem(container, baseID, count, condition, silent);

	if (!new_id)
		return 0;

	if (equipped)
		SetItemEquipped(new_id, true, silent, stick);

	DestroyObject(id);

	return new_id;
}

bool Script::SetItemCount(NetworkID id, unsigned int count) noexcept
{
	bool result = GameFactory::Operate<Item, RETURN_VALIDATED>(id, [id, count](Item* item) {
		if (!count)
			return false;

		if (!item->SetItemCount(count))
			return false;

		if (!GameFactory::Is<ItemList>(item->GetItemContainer()))
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_COUNT>(id, count, false),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

		return true;
	});

	if (result)
		Call<CBI("OnItemCountChange")>(id, count);

	return result;
}

bool Script::SetItemCondition(NetworkID id, double condition) noexcept
{
	bool result = GameFactory::Operate<Item, RETURN_VALIDATED>(id, [id, condition](Item* item) {
		if (condition < 0.00 || condition > 100.0)
			return false;

		auto item_ = DB::Item::Lookup(item->GetBase());

		if (!item_)
			return false;

		if (!item->SetItemCondition(condition))
			return false;

		if (!GameFactory::Is<ItemList>(item->GetItemContainer()))
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_CONDITION>(id, condition, static_cast<unsigned int>(item_->GetHealth() * (condition / 100.0))),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

		return true;
	});

	if (result)
		Call<CBI("OnItemConditionChange")>(id, condition);

	return result;
}

bool Script::SetItemEquipped(NetworkID id, bool equipped, bool silent, bool stick) noexcept
{
	bool result = GameFactory::Operate<Item, RETURN_FACTORY_VALIDATED>(id, [id, equipped, silent, stick](FactoryItem& item) {
		NetworkID container = item->GetItemContainer();

		if (!container)
			return false;

		GameFactory::Free(item);

		return GameFactory::Operate<ItemList, RETURN_FACTORY_VALIDATED>(container, [id, equipped, silent, stick](FactoryItemList& itemlist) {
			return GameFactory::Operate<Item>(id, [&itemlist, id, equipped, silent, stick](Item* item) {
				if (itemlist->IsEquipped(item->GetBase()) == equipped)
					return false;

				item->SetItemEquipped(equipped);
				item->SetItemSilent(silent);

				if (vaultcast_test<Actor>(itemlist))
					Network::Queue({{
						PacketFactory::Create<pTypes::ID_UPDATE_EQUIPPED>(id, equipped, silent, stick),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
					});

				return true;
			});
		});
	});

	if (result)
		Call<CBI("OnItemEquippedChange")>(id, equipped);

	return result;
}

NetworkID Script::CreateContainer(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept
{
	if (!DB::Record::IsValidCoordinate(cell, X, Y, Z))
		return 0;

	NetworkID result = GameFactory::Operate<Container, RETURN_VALIDATED>(GameFactory::Create<Container, FailPolicy::Return>(baseID), [cell, X, Y, Z](Container* container) {
		SetupContainer(container, cell, X, Y, Z);

		Network::Queue({{
			container->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return container->GetNetworkID();
	});

	if (result)
		Call<CBI("OnCreate")>(result);

	return result;
}

NetworkID Script::CreateItemList(NetworkID source, unsigned int baseID) noexcept
{
	NetworkID id = GameFactory::Create<ItemList, FailPolicy::Return>();

	if (!id)
		return id;

	if (source || baseID)
		AddItemList(id, source, baseID);

	Script::Call<CBI("OnCreate")>(id);

	return id;
}

bool Script::DestroyItemList(NetworkID id) noexcept
{
	if (!GameFactory::Exists<ItemList>(id))
		return false;

	Script::Call<CBI("OnDestroy")>(id);

	return GameFactory::Operate<ItemList, BOOL_FACTORY_VALIDATED>(id, [id](FactoryItemList& itemlist) {
		GameFactory::Destroy(itemlist);
	});
}

NetworkID Script::AddItem(NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent) noexcept
{
	auto data = GameFactory::Operate<ItemList, RETURN_FACTORY_VALIDATED>(id, [id, baseID, count, condition, silent](FactoryItemList& itemlist) -> pair<NetworkID, unsigned int> {
		if (!count)
			return {};

		auto diff = itemlist->AddItem(baseID, count, condition, silent);
		unsigned int count;

		GameFactory::Operate<Item>(diff.second, [&itemlist, &diff, &count, silent](Item* item) {
			count = item->GetItemCount();

			if (!vaultcast_test<Container>(itemlist))
				return;

			if (diff.first)
				Network::Queue({{
					item->toPacket(),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
				});
			else
				Network::Queue({{
					PacketFactory::Create<pTypes::ID_UPDATE_COUNT>(diff.second, count, silent),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
				});
		});

		if (diff.first)
			count = 0;

		return {diff.second, count};
	});

	if (data.first)
	{
		if (!data.second)
			Call<CBI("OnCreate")>(data.first);
		else
			Call<CBI("OnItemCountChange")>(data.first, data.second);
	}

	return data.first;
}

void Script::AddItemList(NetworkID id, NetworkID source, unsigned int baseID) noexcept
{
	GameFactory::Operate<ItemList, RETURN_EXPECTED>({id, source}, [id, source, baseID](ItemLists& itemlists) {
		if (!itemlists[0])
			return;

		if (itemlists[1])
		{
			GameFactory::Operate<Item>(itemlists[1]->GetItemList(), [id](Items& items) {
				for (auto item : items)
				{
					AddItem(id, item->GetBase(), item->GetItemCount(), item->GetItemCondition(), item->GetItemSilent());

					if (item->GetItemEquipped())
						EquipItem(id, item->GetBase(), item->GetItemSilent(), item->GetItemStick());
				}
			});
		}
		else if (baseID)
		{
			const auto& items = DB::BaseContainer::Lookup(baseID);

			for (const auto* item : items)
			{
				// FIXME dlc support
				if (item->GetItem() & 0xFF000000)
					continue;

				AddItem(id, item->GetItem(), item->GetCount(), item->GetCondition(), true);
			}
		}
	});
}

unsigned int Script::RemoveItem(NetworkID id, unsigned int baseID, unsigned int count, bool silent) noexcept
{
	auto data = GameFactory::Operate<ItemList, RETURN_FACTORY_VALIDATED>(id, [id, baseID, count, silent](FactoryItemList& itemlist) -> pair<ItemList::RemoveOp, unsigned int> {
		if (!count)
			return {};

		auto diff = itemlist->RemoveItem(baseID, count, silent);

		unsigned int count = get<0>(diff);

		if (!count)
			return {};

		NetworkID update = get<2>(diff);
		unsigned int new_count = 0;

		if (update)
			GameFactory::Operate<Item>(update, [&itemlist, &new_count, update, silent](Item* item) {
				new_count = item->GetItemCount();

				if (vaultcast_test<Container>(itemlist))
					Network::Queue({{
						PacketFactory::Create<pTypes::ID_UPDATE_COUNT>(update, new_count, silent),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
					});
			});

		return {move(diff), new_count};
	});

	if (data.second)
		Call<CBI("OnItemCountChange")>(get<2>(data.first), data.second);

	for (auto id : get<1>(data.first))
		DestroyObject(id);

	return get<0>(data.first);
}

void Script::RemoveAllItems(NetworkID id) noexcept
{
	auto items = GameFactory::Operate<ItemList, RETURN_VALIDATED>(id, [id](ItemList* itemlist) {
		return itemlist->GetItemList();
	});

	for (auto id : items)
		DestroyObject(id);
}

NetworkID Script::CreateActor(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept
{
	if (!DB::Record::IsValidCoordinate(cell, X, Y, Z))
		return 0;

	NetworkID result = GameFactory::Operate<Actor, RETURN_VALIDATED>(GameFactory::Create<Actor, FailPolicy::Return>(baseID), [cell, X, Y, Z](Actor* actor) {
		SetupActor(actor, cell, X, Y, Z);

		Network::Queue({{
			actor->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return actor->GetNetworkID();
	});

	if (result)
		Call<CBI("OnCreate")>(result);

	return result;
}

void Script::SetActorValue(NetworkID id, unsigned char index, double value) noexcept
{
	bool success = GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [id, index, value](Actor* actor) {
		if (!actor->SetActorValue(index, value))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(id, false, index, value),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});

	if (success)
		Call<CBI("OnActorValueChange")>(id, index, value);
}

void Script::SetActorBaseValue(NetworkID id, unsigned char index, double value) noexcept
{
	GameFactory::Operate<Actor, RETURN_VALIDATED>(GameFactory::GetByType(ALL_ACTORS), [id, index, value](Actors& actors) {
		auto it = find_if(actors.begin(), actors.end(), [id](Actor* actor) { return actor->GetNetworkID() == id; });

		if (it == actors.end())
			return;

		unsigned int baseID = (*it)->GetBase();

		if (baseID == PLAYER_BASE)
			return;

		for (auto actor : actors)
			if (actor->GetBase() == baseID)
				if (actor->SetActorBaseValue(index, value))
					Network::Queue({{
						PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(actor->GetNetworkID(), true, index, value),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
					});
	});
}

bool Script::EquipItem(NetworkID id, unsigned int baseID, bool silent, bool stick) noexcept
{
	return GameFactory::Operate<ItemList, RETURN_FACTORY_VALIDATED>(id, [id, baseID, silent, stick](FactoryItemList& itemlist) {
		auto diff = itemlist->EquipItem(baseID, silent, stick);

		if (!diff)
			return false;

		if (vaultcast_test<Actor>(itemlist))
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_EQUIPPED>(diff, true, silent, stick),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

		return true;
	});
}

bool Script::UnequipItem(NetworkID id, unsigned int baseID, bool silent, bool stick) noexcept
{
	return GameFactory::Operate<ItemList, RETURN_FACTORY_VALIDATED>(id, [id, baseID, silent, stick](FactoryItemList& itemlist) {
		auto diff = itemlist->UnequipItem(baseID, silent, stick);

		if (!diff)
			return false;

		if (vaultcast_test<Actor>(itemlist))
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_EQUIPPED>(diff, false, silent, stick),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			});

		return true;
	});
}

bool Script::SetActorMovingAnimation(NetworkID id, unsigned char anim) noexcept
{
	return GameFactory::Operate<Actor, RETURN_FACTORY_VALIDATED>(id, [id, anim](FactoryActor& actor) {
		if (vaultcast_test<Player>(actor))
			return false;

		if (!actor->SetActorMovingAnimation(anim))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), anim, actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), actor->GetActorAlerted(), actor->GetActorSneaking(), false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});
}

bool Script::SetActorWeaponAnimation(NetworkID id, unsigned char anim) noexcept
{
	return GameFactory::Operate<Actor, RETURN_FACTORY_VALIDATED>(id, [id, anim](FactoryActor& actor) {
		if (vaultcast_test<Player>(actor))
			return false;

		if (!actor->SetActorWeaponAnimation(anim))
			return false;

		bool punching = actor->IsActorPunching();
		bool power_punching = actor->IsActorPowerPunching();
		bool firing = actor->IsActorFiring();

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), anim, actor->GetActorAlerted(), actor->GetActorSneaking(), !punching && !power_punching && firing),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});
}

bool Script::SetActorAlerted(NetworkID id, bool alerted) noexcept
{
	bool success = GameFactory::Operate<Actor, RETURN_FACTORY_VALIDATED>(id, [id, alerted](FactoryActor& actor) {
		if (vaultcast_test<Player>(actor))
			return false;

		if (!actor->SetActorAlerted(alerted))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), alerted, actor->GetActorSneaking(), false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});

	if (success)
		Call<CBI("OnActorAlert")>(id, alerted);

	return success;
}

bool Script::SetActorSneaking(NetworkID id, bool sneaking) noexcept
{
	bool success = GameFactory::Operate<Actor, RETURN_FACTORY_VALIDATED>(id, [id, sneaking](FactoryActor& actor) {
		if (vaultcast_test<Player>(actor))
			return false;

		if (!actor->SetActorSneaking(sneaking))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), actor->GetActorAlerted(), sneaking, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});

	if (success)
		Call<CBI("OnActorSneak")>(id, sneaking);

	return success;
}

bool Script::FireWeapon(NetworkID id) noexcept
{
	unsigned int baseID;

	bool success = GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [id, &baseID](Actor* actor) {
		baseID = actor->GetEquippedWeapon();
		auto weapon = DB::Weapon::Lookup(baseID);

		if (!weapon)
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(id, baseID),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});

	if (success)
		Call<CBI("OnActorFireWeapon")>(id, baseID);

	return success;
}

bool Script::PlayIdle(NetworkID id, unsigned int idle) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(id, [id, idle](Actor* actor) {
		auto idle_ = DB::Record::Lookup(idle, "IDLE");

		if (!idle_)
			return false;

		if (!actor->SetActorIdleAnimation(idle))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_IDLE>(id, idle, idle_->GetName()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		return true;
	});
}

void Script::KillActor(NetworkID id, NetworkID actor, unsigned short limbs, signed char cause) noexcept
{
	bool success = GameFactory::Operate<Actor, RETURN_FACTORY_VALIDATED>(id, [id, limbs, cause](FactoryActor& actor) {
		if (!actor->SetActorDead(true, limbs, cause))
			return false;

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, true, limbs, cause),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
		});

		auto player = vaultcast<Player>(actor);

		if (player)
			CreateTimerEx(reinterpret_cast<ScriptFunc>(&Timer_Respawn), player->GetPlayerRespawnTime(), "l", id);

		return true;
	});

	if (success)
		Call<CBI("OnActorDeath")>(id, actor, limbs, cause);
}

bool Script::SetActorBaseRace(NetworkID id, unsigned int race) noexcept
{
	return GameFactory::Operate<Actor, RETURN_FACTORY_VALIDATED>(GameFactory::GetByType(ALL_ACTORS), [id, race](FactoryActors& actors) {
		auto it = find_if(actors.begin(), actors.end(), [id](const FactoryActor& actor) { return actor->GetNetworkID() == id; });

		if (it == actors.end())
			return false;

		unsigned int baseID = (*it)->GetBase();

		if (DB::Record::Lookup(baseID, "CREA"))
			return false;

		if (baseID == PLAYER_BASE)
			return false;

		if (!DB::Race::Lookup(race))
			return false;

		DB::NPC* npc = *DB::NPC::Lookup(baseID);
		unsigned int old_race = npc->GetRace();

		if (old_race == race)
			return false;

		npc->SetRace(race);
		signed int delta_age = DB::Race::Lookup(old_race)->GetAgeDifference(race);
		signed int new_age = DB::Race::Lookup(npc->GetOriginalRace())->GetAgeDifference(race);
		signed int diff_age = DB::Race::Lookup(RACE_CAUCASIAN)->GetAgeDifference(race);

		for (const auto& actor : actors)
			if (actor->GetBase() == baseID)
			{
				actor->SetActorRace(race);
				actor->SetActorAge(new_age);

				if (vaultcast_test<Player>(actor))
					Network::Queue({{
						PacketFactory::Create<pTypes::ID_UPDATE_RACE>(actor->GetNetworkID(), race, diff_age, delta_age),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
					});
				else
					Network::Queue({{
						PacketFactory::Create<pTypes::ID_UPDATE_RACE>(actor->GetNetworkID(), race, new_age, delta_age),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
					});
			}

		return true;
	});
}

bool Script::AgeActorBaseRace(NetworkID id, signed int age) noexcept
{
	return GameFactory::Operate<Actor, RETURN_FACTORY_VALIDATED>(id, [id, age](FactoryActor& actor) {
		if (DB::Record::Lookup(actor->GetBase(), "CREA"))
			return false;

		const DB::Race* race = *DB::Race::Lookup(actor->GetActorRace());
		unsigned int new_race;

		if (age < 0)
		{
			for (signed int i = 0; i > age; --i)
			{
				new_race = race->GetYounger();

				if (!new_race)
					return false;

				race = *DB::Race::Lookup(new_race);
			}
		}
		else if (age > 0)
		{
			for (signed int i = 0; i < age; ++i)
			{
				new_race = race->GetOlder();

				if (!new_race)
					return false;

				race = *DB::Race::Lookup(new_race);
			}
		}
		else
			return true;

		GameFactory::Free(actor);

		return SetActorBaseRace(id, new_race);
	});
}

bool Script::SetActorBaseSex(NetworkID id, bool female) noexcept
{
	return GameFactory::Operate<Actor, RETURN_VALIDATED>(GameFactory::GetByType(ALL_ACTORS), [id, female](Actors& actors) {
		auto it = find_if(actors.begin(), actors.end(), [id](Actor* actor) { return actor->GetNetworkID() == id; });

		if (it == actors.end())
			return false;

		unsigned int baseID = (*it)->GetBase();

		if (DB::Record::Lookup(baseID, "CREA"))
			return false;

		if (baseID == PLAYER_BASE)
			return false;

		DB::NPC* npc = *DB::NPC::Lookup(baseID);
		bool old_female = npc->IsFemale();

		if (old_female == female)
			return false;

		npc->SetFemale(female);

		for (auto actor : actors)
			if (actor->GetBase() == baseID)
			{
				actor->SetActorFemale(female);

				Network::Queue({{
					PacketFactory::Create<pTypes::ID_UPDATE_SEX>(actor->GetNetworkID(), female),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
				});
			}

		return true;
	});

	return true;
}

void Script::SetPlayerRespawnTime(NetworkID id, unsigned int respawn) noexcept
{
	GameFactory::Operate<Player, RETURN_VALIDATED>(id, [respawn](Player* player) {
		player->SetPlayerRespawnTime(respawn);
	});
}

void Script::SetPlayerSpawnCell(NetworkID id, unsigned int cell) noexcept
{
	GameFactory::Operate<Player, RETURN_VALIDATED>(id, [id, cell](Player* player) {
		auto cell_ = DB::Exterior::Lookup(cell);

		if (!cell_)
		{
			auto record = DB::Record::Lookup(cell, "CELL");

			if (!record)
				return;

			if (player->SetPlayerSpawnCell(cell))
				Network::Queue({
					{
						PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(id, record->GetName(), true),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
					},
					{
						PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, Player::CellContext{{cell, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}}, true),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
					}
				});
		}
		else if (player->SetPlayerSpawnCell(cell))
			Network::Queue({
				{
					PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, cell_->GetWorld(), cell_->GetX(), cell_->GetY(), true),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
				},
				{
					PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, cell_->GetAdjacents(), true),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
				}
			});
	});
}

void Script::SetPlayerConsoleEnabled(NetworkID id, bool enabled) noexcept
{
	GameFactory::Operate<Player, RETURN_VALIDATED>(id, [id, enabled](Player* player) {
		if (player->SetPlayerConsoleEnabled(enabled))
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_CONSOLE>(id, enabled),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()}
			});
	});
}

bool Script::AttachWindow(NetworkID id, NetworkID window) noexcept
{
	if (!GameFactory::Operate<Window, RETURN_VALIDATED>(window, [](Window* window) {
		return !window->GetParentWindow();
	})) return false;

	if (!GameFactory::Operate<Player, RETURN_VALIDATED>(id, [window](Player* player) {
		return player->AttachWindow(window);
	})) return false;

	vector<NetworkID> additions;
	Window::CollectChilds(window, additions);

	NetworkResponse response;

	for (auto id_ : additions)
		response.emplace_back(
			GameFactory::Get<Window>(id_)->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
		);

	Network::Queue(move(response));

	return true;
}

bool Script::DetachWindow(NetworkID id, NetworkID window) noexcept
{
	if (!GameFactory::Operate<Window, RETURN_VALIDATED>(window, [window](Window*) {
		return !IsChatbox(window);
	})) return false;

	if (!GameFactory::Operate<Player, RETURN_VALIDATED>(id, [window](Player* player) {
		return player->DetachWindow(window);
	})) return false;

	vector<NetworkID> deletions;
	Window::CollectChilds(window, deletions);
	reverse(deletions.begin(), deletions.end()); // reverse so the order of deletion is valid

	NetworkResponse response;

	for (auto id_ : deletions)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_WINDOW_REMOVE>(id_),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()
		);

	Network::Queue(move(response));

	return true;
}

void Script::ForceWindowMode(NetworkID id, bool enabled) noexcept
{
	GameFactory::Operate<Player, RETURN_VALIDATED>(id, [id, enabled](Player*) {
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WMODE>(enabled),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()}
		});
	});
}

NetworkID Script::GetWindowParent(NetworkID id) noexcept
{
	return GameFactory::Operate<Window, RETURN_VALIDATED>(id, [id](Window* window) {
		return window->GetParentWindow();
	});
}

NetworkID Script::GetWindowRoot(NetworkID id) noexcept
{
	return GameFactory::Operate<Window, RETURN_FACTORY_VALIDATED>(id, [id](FactoryWindow& window) {
		for (NetworkID parent; (parent = window->GetParentWindow()); GameFactory::Free(window), window = GameFactory::Get<Window>(parent).get());

		return window->GetNetworkID();
	});
}

unsigned int Script::GetWindowChildCount(NetworkID id) noexcept
{
	return GameFactory::Operate<Window, RETURN_VALIDATED>(id, [id](Window*) {
		const auto& childs = Window::GetChilds();
		return childs.count(id) ? childs.find(id)->second.size() : 0;
	});
}

unsigned int Script::GetWindowChildList(NetworkID id, NetworkID** data) noexcept
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<Window, RETURN_VALIDATED>(id, [id, data](Window*) {
		const auto& childs_ = Window::GetChilds();

		if (!childs_.count(id))
			return 0u;

		const auto& childs = childs_.find(id)->second;
		_data.assign(childs.begin(), childs.end());
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	});
}

void Script::GetWindowPos(NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y) noexcept
{
	*X = 0.0;
	*Y = 0.0;
	*offset_X = 0.0;
	*offset_Y = 0.0;

	GameFactory::Operate<Window, RETURN_VALIDATED>(id, [X, Y, offset_X, offset_Y](Window* window) {
		const auto& pos = window->GetPos();
		*X = get<0>(pos);
		*Y = get<1>(pos);
		*offset_X = get<2>(pos);
		*offset_Y = get<3>(pos);
	});
}

void Script::GetWindowSize(NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y) noexcept
{
	*X = 0.0;
	*Y = 0.0;
	*offset_X = 0.0;
	*offset_Y = 0.0;

	GameFactory::Operate<Window, RETURN_VALIDATED>(id, [X, Y, offset_X, offset_Y](Window* window) {
		const auto& size = window->GetSize();
		*X = get<0>(size);
		*Y = get<1>(size);
		*offset_X = get<2>(size);
		*offset_Y = get<3>(size);
	});
}

bool Script::GetWindowVisible(NetworkID id) noexcept
{
	return GameFactory::Operate<Window, RETURN_VALIDATED>(id, [id](Window* window) {
		return window->GetVisible();
	});
}

bool Script::GetWindowLocked(NetworkID id) noexcept
{
	return GameFactory::Operate<Window, RETURN_VALIDATED>(id, [id](Window* window) {
		return window->GetLocked();
	});
}

const char* Script::GetWindowText(NetworkID id) noexcept
{
	static string text;

	return GameFactory::Operate<Window, BOOL_VALIDATED>(id, [](Window* window) {
		text.assign(window->GetText());
	}) ? text.c_str() : "";
}

unsigned int Script::GetEditMaxLength(NetworkID id) noexcept
{
	return GameFactory::Operate<Edit, RETURN_VALIDATED>(id, [id](Edit* edit) {
		return edit->GetMaxLength();
	});
}

const char* Script::GetEditValidation(NetworkID id) noexcept
{
	static string validation;

	return GameFactory::Operate<Edit, BOOL_VALIDATED>(id, [](Edit* edit) {
		validation.assign(edit->GetValidation());
	}) ? validation.c_str() : "";
}

bool Script::GetCheckboxSelected(NetworkID id) noexcept
{
	return GameFactory::Operate<Checkbox, RETURN_VALIDATED>(id, [id](Checkbox* checkbox) {
		return checkbox->GetSelected();
	});
}

bool Script::GetRadioButtonSelected(NetworkID id) noexcept
{
	return GameFactory::Operate<RadioButton, RETURN_VALIDATED>(id, [id](RadioButton* radiobutton) {
		return radiobutton->GetSelected();
	});
}

unsigned int Script::GetRadioButtonGroup(NetworkID id) noexcept
{
	return GameFactory::Operate<RadioButton, RETURN_VALIDATED>(id, [id](RadioButton* radiobutton) {
		return radiobutton->GetGroup();
	});
}

bool Script::GetListMultiSelect(NetworkID id) noexcept
{
	return GameFactory::Operate<List, RETURN_VALIDATED>(id, [id](List* list) {
		return list->GetMultiSelect();
	});
}

unsigned int Script::GetListItemCount(NetworkID id) noexcept
{
	return GameFactory::Operate<List, RETURN_VALIDATED>(id, [id](List* list) {
		return list->GetItemList().size();
	});
}

unsigned int Script::GetListItemList(NetworkID id, NetworkID** data) noexcept
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<List, RETURN_VALIDATED>(id, [id, data](List* list) {
		const auto& listitems = list->GetItemList();
		_data.assign(listitems.begin(), listitems.end());
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	});
}

unsigned int Script::GetListSelectedItemCount(NetworkID id) noexcept
{
	return GameFactory::Operate<List, RETURN_VALIDATED>(id, [id](List* list) {
		return count_if(list->GetItemList().begin(), list->GetItemList().end(), [](NetworkID listitem) { return GameFactory::Get<ListItem>(listitem)->GetSelected(); });
	});
}

unsigned int Script::GetListSelectedItemList(NetworkID id, NetworkID** data) noexcept
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<List, RETURN_VALIDATED>(id, [id, data](List* list) {
		const auto& listitems = list->GetItemList();
		_data.clear();
		copy_if(listitems.begin(), listitems.end(), back_inserter(_data), [](NetworkID listitem) { return GameFactory::Get<ListItem>(listitem)->GetSelected(); });
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	});
}

NetworkID Script::GetListItemContainer(NetworkID id) noexcept
{
	return GameFactory::Operate<ListItem, RETURN_VALIDATED>(id, [id](ListItem* listitem) {
		return listitem->GetItemContainer();
	});
}

bool Script::GetListItemSelected(NetworkID id) noexcept
{
	return GameFactory::Operate<ListItem, RETURN_VALIDATED>(id, [id](ListItem* listitem) {
		return listitem->GetSelected();
	});
}

const char* Script::GetListItemText(NetworkID id) noexcept
{
	static string text;

	return GameFactory::Operate<ListItem, BOOL_VALIDATED>(id, [](ListItem* listitem) {
		text.assign(listitem->GetText());
	}) ? text.c_str() : "";
}

NetworkID (Script::CreateWindow)(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept
{
	NetworkID id = GameFactory::Create<Window, FailPolicy::Return>(posX, posY, sizeX, sizeY, visible, locked, text);

	if (!id)
		return id;

	Call<CBI("OnCreate")>(id);

	return id;
}

bool Script::DestroyWindow(NetworkID id) noexcept
{
	NetworkID root = GetWindowRoot(id);

	if (!root || IsChatbox(id))
		return false;

	vector<NetworkID> deletions;
	Window::CollectChilds(id, deletions);
	reverse(deletions.begin(), deletions.end()); // reverse so the order of deletion is valid

	for (auto id : deletions)
	{
		if (!GameFactory::Exists<Window>(id))
			continue;

		Call<CBI("OnDestroy")>(id);

		if (!GameFactory::Exists<Window>(id))
			continue;

		auto players = Player::GetWindowPlayers(root);
		vector<RakNetGUID> guids(Client::GetNetworkList(players));

		if (root == id)
			for (auto id : players)
				GameFactory::Operate<Player, RETURN_VALIDATED>(id, [root](Player* player) {
					player->DetachWindow(root);
				});

		if (!guids.empty())
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_WINDOW_REMOVE>(id),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
			});

		GameFactory::Destroy(id);
	}

	return true;
}

bool Script::AddChildWindow(NetworkID id, NetworkID child) noexcept
{
	if (!GameFactory::Operate<Window>({id, child}, [id, child](Windows& windows) {
		if (IsChatbox(id) || IsChatbox(child))
			return false;

		if (windows[1]->GetParentWindow())
			return false;

		windows[1]->SetParentWindow(windows[0]);

		return true;
	})) return false;

	vector<NetworkID> additions;
	Window::CollectChilds(child, additions);

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
	{
		NetworkResponse response;

		for (auto id : additions)
			response.emplace_back(
				GameFactory::Get<Window>(id)->toPacket(),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids
			);

		Network::Queue(move(response));
	}

	return true;
}

bool Script::RemoveChildWindow(NetworkID id, NetworkID child) noexcept
{
	if (!GameFactory::Operate<Window>({id, child}, [id](Windows& windows) {
		if (windows[1]->GetParentWindow() != id)
			return false;

		windows[1]->SetParentWindow(nullptr);

		return true;
	})) return false;

	vector<NetworkID> deletions;
	Window::CollectChilds(child, deletions);
	reverse(deletions.begin(), deletions.end()); // reverse so the order of deletion is valid

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
	{
		NetworkResponse response;

		for (auto id : deletions)
			response.emplace_back(
				PacketFactory::Create<pTypes::ID_WINDOW_REMOVE>(id),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids
			);

		Network::Queue(move(response));
	}

	return true;
}

bool Script::SetWindowPos(NetworkID id, double X, double Y, double offset_X, double offset_Y) noexcept
{
	if (!GameFactory::Operate<Window, RETURN_VALIDATED>(id, [X, Y, offset_X, offset_Y](Window* window) {
		return window->SetPos(X, Y, offset_X, offset_Y);
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WPOS>(id, tuple<double, double, double, double>{X, Y, offset_X, offset_Y}),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	return true;
}

bool Script::SetWindowSize(NetworkID id, double X, double Y, double offset_X, double offset_Y) noexcept
{
	if (!GameFactory::Operate<Window, RETURN_VALIDATED>(id, [X, Y, offset_X, offset_Y](Window* window) {
		return window->SetSize(X, Y, offset_X, offset_Y);
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WSIZE>(id, tuple<double, double, double, double>{X, Y, offset_X, offset_Y}),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	return true;
}

bool Script::SetWindowVisible(NetworkID id, bool visible) noexcept
{
	if (!GameFactory::Operate<Window, BOOL_VALIDATED>(id, [visible](Window* window) {
		window->SetVisible(visible);
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WVISIBLE>(id, visible),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	return true;
}

bool Script::SetWindowLocked(NetworkID id, bool locked) noexcept
{
	if (!GameFactory::Operate<Window, BOOL_VALIDATED>(id, [locked](Window* window) {
		window->SetLocked(locked);
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WLOCKED>(id, locked),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	return true;
}

bool Script::SetWindowText(NetworkID id, const char* text) noexcept
{
	if (!GameFactory::Operate<Window, RETURN_FACTORY_VALIDATED>(id, [text](FactoryWindow& window) {
		auto edit = vaultcast<Edit>(window);

		if (edit && edit->GetMaxLength() < strlen(text))
			return false;

		window->SetText(text);

		return true;
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WTEXT>(id, text),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	Call<CBI("OnWindowTextChange")>(id, 0ull, text);

	return true;
}

NetworkID Script::CreateButton(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept
{
	NetworkID id = GameFactory::Create<Button, FailPolicy::Return>(posX, posY, sizeX, sizeY, visible, locked, text);

	if (!id)
		return id;

	Call<CBI("OnCreate")>(id);

	return id;
}

NetworkID Script::CreateText(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept
{
	NetworkID id = GameFactory::Create<Text, FailPolicy::Return>(posX, posY, sizeX, sizeY, visible, locked, text);

	if (!id)
		return id;

	Call<CBI("OnCreate")>(id);

	return id;
}

NetworkID Script::CreateEdit(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept
{
	NetworkID id = GameFactory::Create<Edit, FailPolicy::Return>(posX, posY, sizeX, sizeY, visible, locked, text);

	if (!id)
		return id;

	Call<CBI("OnCreate")>(id);

	return id;
}

bool Script::SetEditMaxLength(NetworkID id, unsigned int length) noexcept
{
	bool text_updated = false;
	string new_text;

	if (!GameFactory::Operate<Edit, BOOL_VALIDATED>(id, [length, &text_updated, &new_text](Edit* edit) {
		edit->SetMaxLength(length);

		new_text = edit->GetText();

		if (new_text.length() > length)
		{
			new_text.resize(length);
			edit->SetText(new_text);
			text_updated = true;
		}
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
	{
		NetworkResponse response;

		if (text_updated)
			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_WTEXT>(id, new_text),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids
			);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_WMAXLEN>(id, length),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids
		);

		Network::Queue(move(response));
	}

	if (text_updated)
		Call<CBI("OnWindowTextChange")>(id, 0ull, new_text.c_str());

	return true;
}

bool Script::SetEditValidation(NetworkID id, const char* validation) noexcept
{
	if (!GameFactory::Operate<Edit, BOOL_VALIDATED>(id, [validation](Edit* edit) {
		regex verify(validation);
		edit->SetValidation(validation);
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WVALID>(id, validation),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	return true;
}

NetworkID Script::CreateCheckbox(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept
{
	NetworkID id = GameFactory::Create<Checkbox, FailPolicy::Return>(posX, posY, sizeX, sizeY, visible, locked, text);

	if (!id)
		return id;

	Call<CBI("OnCreate")>(id);

	return id;
}

bool Script::SetCheckboxSelected(NetworkID id, bool selected) noexcept
{
	if (!GameFactory::Operate<Checkbox, BOOL_VALIDATED>(id, [selected](Checkbox* checkbox) {
		checkbox->SetSelected(selected);
	})) return false;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WSELECTED>(id, selected),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	Call<CBI("OnCheckboxSelect")>(id, 0ull, selected);

	return true;
}

NetworkID Script::CreateRadioButton(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept
{
	NetworkID id = GameFactory::Create<RadioButton, FailPolicy::Return>(posX, posY, sizeX, sizeY, visible, locked, text);

	if (!id)
		return id;

	Call<CBI("OnCreate")>(id);

	return id;
}

bool Script::SetRadioButtonSelected(NetworkID id, bool selected) noexcept
{
	if (!GameFactory::Exists<RadioButton>(id))
		return false;

	unsigned int group = GameFactory::Operate<RadioButton, RETURN_VALIDATED>(id, [selected](RadioButton* radiobutton) {
		radiobutton->SetSelected(selected);
		return radiobutton->GetGroup();
	});

	NetworkID previous = selected ? GameFactory::Operate<RadioButton, RETURN_VALIDATED>(GameFactory::GetByType(ID_RADIOBUTTON), [group, id](RadioButtons& radiobuttons) {
		for (auto radiobutton : radiobuttons)
			if (radiobutton->GetGroup() == group && radiobutton->GetSelected() && radiobutton->GetNetworkID() != id)
			{
				radiobutton->SetSelected(false);
				return radiobutton->GetNetworkID();
			}

		return 0ull;
	}) : 0;

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WRSELECTED>(id, previous, selected),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	Call<CBI("OnRadioButtonSelect")>(id, previous, 0ull);

	return true;
}

bool Script::SetRadioButtonGroup(NetworkID id, unsigned int group)
{
	if (!GameFactory::Exists<RadioButton>(id))
		return false;

	pair<unsigned int, bool> data = GameFactory::Operate<RadioButton, RETURN_VALIDATED>(id, [](RadioButton* radiobutton) {
		return make_pair(radiobutton->GetGroup(), radiobutton->GetSelected());
	});

	if (data.first == group)
		return false;

	if (data.second)
		if (!GameFactory::Operate<RadioButton, RETURN_VALIDATED>(GameFactory::GetByType(ID_RADIOBUTTON), [group](RadioButtons& radiobuttons) {
			for (auto radiobutton : radiobuttons)
				if (radiobutton->GetGroup() == group && radiobutton->GetSelected())
					return false;

			return true;
		})) return false;

	GameFactory::Operate<RadioButton, RETURN_VALIDATED>(id, [group](RadioButton* radiobutton) {
		radiobutton->SetGroup(group);
	});

	NetworkID root = GetWindowRoot(id);
	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

	if (!guids.empty())
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WGROUP>(id, group),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
		});

	return true;
}

NetworkID Script::CreateList(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept
{
	NetworkID id = GameFactory::Create<List, FailPolicy::Return>(posX, posY, sizeX, sizeY, visible, locked, text);

	if (!id)
		return id;

	Call<CBI("OnCreate")>(id);

	return id;
}

bool Script::SetListMultiSelect(NetworkID id, bool multiselect)
{
	return GameFactory::Operate<List, RETURN_VALIDATED>(id, [id, multiselect](List* list) {
		if (list->GetMultiSelect() == multiselect)
			return false;

		if (!multiselect)
		{
			NetworkID* selected;
			unsigned int count = GetListSelectedItemList(id, &selected);

			while (count > 1)
			{
				SetListItemSelected(selected[count - 1], false);
				--count;
			}
		}

		list->SetMultiSelect(multiselect);

		NetworkID root = GetWindowRoot(id);
		vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

		if (!guids.empty())
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_WLMULTI>(id, multiselect),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
			});

		return true;
	});
}

NetworkID Script::AddListItem(NetworkID id, const char* text) noexcept
{
	if (!*text)
		return 0ull;

	NetworkID listitem = GameFactory::Operate<List, RETURN_VALIDATED>(id, [id, text](List* list) {
		return GameFactory::Operate<ListItem>(GameFactory::Create<ListItem, FailPolicy::Return>(), [id, text, &list](ListItem* listitem) {
			NetworkID listitem_id = listitem->GetNetworkID();
			list->AddItem(listitem_id);
			listitem->SetText(text);

			NetworkID root = GetWindowRoot(id);
			vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

			if (!guids.empty())
				Network::Queue({{
					listitem->toPacket(),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
				});

			return listitem_id;
		});
	});

	if (!listitem)
		return listitem;

	Call<CBI("OnCreate")>(listitem);

	return listitem;
}

bool Script::RemoveListItem(NetworkID id) noexcept
{
	NetworkID list = GameFactory::Operate<ListItem, RETURN_VALIDATED>(id, [](ListItem* listitem) {
		return listitem->GetItemContainer();
	});

	if (!list)
		return false;

	Call<CBI("OnDestroy")>(id);

	return GameFactory::Operate<List, BOOL_VALIDATED>(list, [id](List* list) {
		list->RemoveItem(id);

		NetworkID root = GetWindowRoot(list->GetNetworkID());
		vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

		if (!guids.empty())
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_LISTITEM_REMOVE>(id),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
			});

		GameFactory::Destroy(id);
	});
}

NetworkID Script::SetListItemContainer(NetworkID id, NetworkID container) noexcept
{
	const string* text;
	bool selected;

	if (!GameFactory::Operate<ListItem, RETURN_VALIDATED>(id, [id, container, &text, &selected](ListItem* listitem) {
		if (listitem->GetItemContainer() == container)
			return false;

		text = &listitem->GetText();
		selected = listitem->GetSelected();
		return true;
	})) return 0;

	NetworkID new_id = AddListItem(container, text->c_str());

	if (!new_id)
		return 0;

	if (selected)
		SetListItemSelected(new_id, true);

	RemoveListItem(id);

	return new_id;
}

bool Script::SetListItemSelected(NetworkID id, bool selected) noexcept
{
	NetworkID list = GameFactory::Operate<ListItem, RETURN_VALIDATED>(id, [selected](ListItem* listitem) {
		return listitem->GetSelected() != selected ? listitem->GetItemContainer() : 0ull;
	});

	if (!list)
		return false;

	NetworkID previous = 0;

	bool success = GameFactory::Operate<List, BOOL_VALIDATED>(list, [id, selected, &previous](List* list) {
		if (selected && !list->GetMultiSelect())
		{
			NetworkID* selected;

			if (GetListSelectedItemList(list->GetNetworkID(), &selected))
				previous = selected[0];
		}

		GameFactory::Operate<ListItem>(id, [selected](ListItem* listitem) {
			listitem->SetSelected(selected);
		});

		NetworkID root = GetWindowRoot(list->GetNetworkID());
		vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

		if (!guids.empty())
		{
			NetworkResponse response;

			if (previous)
				response.emplace_back(
					PacketFactory::Create<pTypes::ID_UPDATE_WLSELECTED>(previous, false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids);

			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_WLSELECTED>(id, selected),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids);

			Network::Queue(move(response));
		}
	});

	if (success)
	{
		if (previous)
			Call<CBI("OnListItemSelect")>(previous, 0ull, false);

		Call<CBI("OnListItemSelect")>(id, 0ull, selected);
	}

	return success;
}

bool Script::SetListItemText(NetworkID id, const char* text) noexcept
{
	if (!*text)
		return false;

	NetworkID list = GameFactory::Operate<ListItem, RETURN_VALIDATED>(id, [](ListItem* listitem) {
		return listitem->GetItemContainer();
	});

	if (!list)
		return false;

	return GameFactory::Operate<List, BOOL_VALIDATED>(list, [text, id](List* list) {
		return GameFactory::Operate<ListItem>(id, [text, id, &list](ListItem* listitem) {
			listitem->SetText(text);

			NetworkID root = GetWindowRoot(list->GetNetworkID());
			vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root)));

			if (!guids.empty())
				Network::Queue({{
					PacketFactory::Create<pTypes::ID_UPDATE_WLTEXT>(id, text),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids}
				});
		});
	});
}
