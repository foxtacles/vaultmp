#include "Script.h"
#include "Timer.h"

using namespace std;
using namespace RakNet;
using namespace Values;

vector<Script*> Script::scripts;

unordered_map<NetworkID, unique_ptr<ItemList>> Script::scriptIL;
pair<chrono::system_clock::time_point, double> Script::gameTime;
unsigned int Script::gameWeather;

template<typename... Types>
constexpr char TypeString<Types...>::value[];
constexpr ScriptFunctionData Script::functions[];

Script::Script(char* path)
{
	FILE* file = fopen(path, "rb");

	if (file == nullptr)
		throw VaultException("Script not found: %s", path).stacktrace();

	fclose(file);

#ifdef __WIN32__
	if (strstr(path, ".dll"))
#else
	if (strstr(path, ".so"))
#endif
	{
		lib_t handle;
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

#ifdef __WIN32__
	#define GetScript(a,b) (b = (decltype(b)) GetProcAddress(this->lib,a))
#else
	#define GetScript(a,b) (b = (decltype(b)) dlsym(this->lib,a))
#endif

			GetScript("exec", fexec);

			if (!fexec)
				throw VaultException("Could not find exec() callback in: %s", path).stacktrace();

			const char* vaultprefix;
			GetScript("vaultprefix", vaultprefix);
			string vpf(vaultprefix);

			GetScript("OnSpawn", fOnSpawn);
			GetScript("OnLockChange", fOnLockChange);
			GetScript("OnCellChange", fOnCellChange);
			GetScript("OnContainerItemChange", fOnContainerItemChange);
			GetScript("OnActorValueChange", fOnActorValueChange);
			GetScript("OnActorBaseValueChange", fOnActorBaseValueChange);
			GetScript("OnActorAlert", fOnActorAlert);
			GetScript("OnActorSneak", fOnActorSneak);
			GetScript("OnActorDeath", fOnActorDeath);
			GetScript("OnActorEquipItem", fOnActorEquipItem);
			GetScript("OnActorUnequipItem", fOnActorUnequipItem);
			GetScript("OnActorDropItem", fOnActorDropItem);
			GetScript("OnActorPickupItem", fOnActorPickupItem);
			GetScript("OnActorPunch", fOnActorPunch);
			GetScript("OnActorFireWeapon", fOnActorFireWeapon);
			GetScript("OnPlayerDisconnect", fOnPlayerDisconnect);
			GetScript("OnPlayerRequestGame", fOnPlayerRequestGame);
			GetScript("OnPlayerChat", fOnPlayerChat);
			GetScript("OnClientAuthenticate", fOnClientAuthenticate);
			GetScript("OnGameYearChange", fOnGameYearChange);
			GetScript("OnGameMonthChange", fOnGameMonthChange);
			GetScript("OnGameDayChange", fOnGameDayChange);
			GetScript("OnGameHourChange", fOnGameHourChange);
			GetScript("OnServerInit", fOnServerInit);
			GetScript("OnServerExit", fOnServerExit);

			auto SetScript = [this](const char* name, ScriptFunctionPointer func)
			{
#ifdef __WIN32__
				auto addr = GetProcAddress(this->lib, name);
#else
				auto addr = dlsym(this->lib, name);
#endif
				if (addr)
					*reinterpret_cast<decltype(func.addr)*>(addr) = func.addr;
				else
					printf("Script function pointer not found: %s\n", name);
			};

			for (const auto& function : functions)
				SetScript(string(vpf + function.name).c_str(), function.func);
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

#ifdef __WIN32__
void Script::LoadScripts(char* scripts, char*)
#else
void Script::LoadScripts(char* scripts, char* base)
#endif
{
	char* token = strtok(scripts, ",");

	try
	{
		while (token != nullptr)
		{
#ifdef __WIN32__
			Script* script = new Script(token);
#else
			char path[MAX_PATH];
			snprintf(path, sizeof(path), "%s/%s", base, token);
			Script* script = new Script(path);
#endif
			Script::scripts.emplace_back(script);
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

	gameTime.first = chrono::system_clock::now();
	gameTime.second = 1.0;
	CreateTimer(&Timer_GameTime, 1000);

	gameWeather = DEFAULT_WEATHER;

	auto object_init = [](FactoryObject<Object>& object, const DB::Reference* reference)
	{
		const auto& pos = reference->GetPos();
		const auto& angle = reference->GetAngle();
		auto cell = reference->GetCell();
		auto lock = reference->GetLock();
		object->SetNetworkPos(Axis_X, get<0>(pos));
		object->SetNetworkPos(Axis_Y, get<1>(pos));
		object->SetNetworkPos(Axis_Z, get<2>(pos));
		object->SetGamePos(Axis_X, get<0>(pos));
		object->SetGamePos(Axis_Y, get<1>(pos));
		object->SetGamePos(Axis_Z, get<2>(pos));
		object->SetAngle(Axis_X, get<0>(angle));
		object->SetAngle(Axis_Y, get<1>(angle));
		object->SetAngle(Axis_Z, get<2>(angle));

		auto exterior = DB::Exterior::Lookup(cell);

		if (exterior)
		{
			auto match_exterior = DB::Exterior::Lookup(exterior->GetWorld(), get<0>(pos), get<1>(pos));

#ifdef VAULTMP_DEBUG
/*
			if (exterior->GetBase() != match_exterior->GetBase())
				debug.print("Error matching position with cell: ", hex, object->GetReference(), " relocating from ", dec, exterior->GetX(), ",", exterior->GetY(), " to ",  match_exterior->GetX(), ",", match_exterior->GetY());
*/
#endif
			cell = match_exterior->GetBase();
		}

		object->SetNetworkCell(cell);
		object->SetGameCell(cell);
		object->SetLockLevel(lock);
	};

	auto objects = DB::Reference::Lookup("CONT");

	for (const auto* reference : objects)
	{
		// FIXME dlc support
		if (reference->GetReference() & 0xFF000000)
			continue;

		auto container = GameFactory::GetObject<Container>(GameFactory::CreateInstance(ID_CONTAINER, reference->GetReference(), reference->GetBase())).get();
		object_init(container, reference);
	}

	objects = DB::Reference::Lookup("DOOR");

	for (const auto* reference : objects)
	{
		// FIXME dlc support
		if (reference->GetReference() & 0xFF000000)
			continue;

		auto door = GameFactory::GetObject(GameFactory::CreateInstance(ID_OBJECT, reference->GetReference(), reference->GetBase())).get();
		object_init(door, reference);
	}

	objects = DB::Reference::Lookup("TERM");

	for (const auto* reference : objects)
	{
		// FIXME dlc support
		if (reference->GetReference() & 0xFF000000)
			continue;

		auto terminal = GameFactory::GetObject(GameFactory::CreateInstance(ID_OBJECT, reference->GetReference(), reference->GetBase())).get();
		object_init(terminal, reference);
		terminal->SetLockLevel(DB::Terminal::Lookup(reference->GetBase())->GetLock());
	}
}

void Script::Run()
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
			script->fexec();
		else
		{
			cell ret;
			int err = PAWN::Exec(script->amx, &ret, AMX_EXEC_MAIN);

			if (err != AMX_ERR_NONE)
				throw VaultException("PAWN script error (%d): \"%s\"", err, aux_StrError(err)).stacktrace();
		}
	}
}

void Script::UnloadScripts()
{
	for (Script* script : scripts)
		delete script;

	Timer::TerminateAll();
	Public::DeleteAll();
	scripts.clear();
	scriptIL.clear();
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
					params.emplace_back(va_arg(args, const char*));
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

NetworkID Script::CreateTimer(ScriptFunc timer, unsigned int interval)
{
	Timer* t = new Timer(timer, string(), vector<boost::any>(), interval);
	return t->GetNetworkID();
}

NetworkID Script::CreateTimerEx(ScriptFunc timer, unsigned int interval, const char* def, ...)
{
	vector<boost::any> params;

	va_list args;
	va_start(args, def);
	GetArguments(params, args, def);
	va_end(args);

	Timer* t = new Timer(timer, def, params, interval);
	return t->GetNetworkID();
}

NetworkID Script::CreateTimerPAWN(ScriptFuncPAWN timer, AMX* amx, unsigned int interval)
{
	Timer* t = new Timer(timer, amx, string(), vector<boost::any>(), interval);
	return t->GetNetworkID();
}

NetworkID Script::CreateTimerPAWNEx(ScriptFuncPAWN timer, AMX* amx, unsigned int interval, const char* def, const vector<boost::any>& args)
{
	Timer* t = new Timer(timer, amx, string(def), args, interval);
	return t->GetNetworkID();
}

void Script::SetupObject(FactoryObject<Object>& object, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z)
{
	if (reference)
	{
		object->SetNetworkCell(reference->GetNetworkCell());
		object->SetNetworkPos(Axis_X, reference->GetNetworkPos(Axis_X));
		object->SetNetworkPos(Axis_Y, reference->GetNetworkPos(Axis_Y));
		object->SetNetworkPos(Axis_Z, reference->GetNetworkPos(Axis_Z));
	}
	else
		SetCell_(object->GetNetworkID(), cell, X, Y, Z, true);
}

void Script::SetupItem(FactoryObject<Item>& item, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z)
{
	SetupObject(item, reference, cell, X, Y, Z);

	item->SetItemCount(1);
	item->SetItemCondition(100.0);
}

void Script::SetupContainer(FactoryObject<Container>& container, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z)
{
	SetupObject(container, reference, cell, X, Y, Z);
}

void Script::SetupActor(FactoryObject<Actor>& actor, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z)
{
	SetupContainer(actor, reference, cell, X, Y, Z);

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

void Script::KillTimer(NetworkID id)
{
	if (!id)
		id = Timer::LastTimer();

	Timer::Terminate(id);
}

void Script::MakePublic(ScriptFunc _public, const char* name, const char* def)
{
	new Public(_public, string(name), string(def));
}

void Script::MakePublicPAWN(ScriptFuncPAWN _public, AMX* amx, const char* name, const char* def)
{
	new Public(_public, amx, string(name), string(def));
}

unsigned long long Script::CallPublic(const char* name, ...)
{
	vector<boost::any> params;
	string def = Public::GetDefinition(name);

	va_list args;
	va_start(args, name);
	GetArguments(params, args, def);
	va_end(args);

	return Public::Call(name, params);
}

unsigned long long Script::CallPublicPAWN(const char* name, const vector<boost::any>& args)
{
	return Public::Call(name, args);
}

unsigned long long Script::Timer_Respawn(NetworkID id)
{
	if (!GameFactory::GetObject<Player>(id))
	{
		KillTimer(); // Player has already left the server
		return 0;
	}

	RakNetGUID guid = Client::GetClientFromPlayer(id)->GetGUID();

	RemoveAllItems(id);

	for (const auto& item : Player::default_items)
	{
		AddItem(id, item.first, get<0>(item.second), get<1>(item.second), get<3>(item.second));

		if (get<2>(item.second))
			EquipItem(id, item.first, get<3>(item.second), get<4>(item.second));
	}

	const auto& values = Player::f3_default_values;

	for (const auto& value : values)
	{
		SetActorBaseValue(id, value.first, value.second.first);
		SetActorValue(id, value.first, value.second.second);
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, false, 0, 0),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid)
	});

	KillTimer();

	return 1;
}

unsigned long long Script::Timer_GameTime()
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	gameTime.first += chrono::milliseconds(static_cast<unsigned long long>(1000ull * gameTime.second));

	t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();
	TM _tm_new;
	gmtime64_r(&t, &_tm_new);

	if (_tm.tm_year != _tm_new.tm_year)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, _tm_new.tm_year),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		Script::OnGameYearChange(_tm_new.tm_year + 1900);
	}

	if (_tm.tm_mon != _tm_new.tm_mon)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, _tm_new.tm_mon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		Script::OnGameMonthChange(_tm_new.tm_mon);
	}

	if (_tm.tm_mday != _tm_new.tm_mday)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, _tm_new.tm_mday),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		Script::OnGameDayChange(_tm_new.tm_mday);
	}

	if (_tm.tm_hour != _tm_new.tm_hour)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, _tm_new.tm_hour),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		Script::OnGameHourChange(_tm_new.tm_hour);
	}

	return 1;
}

void Script::OnSpawn(NetworkID id)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnSpawn)
				script->fOnSpawn(id);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnSpawn"))
			PAWN::Call(script->amx, "OnSpawn", "l", 0, id);
	}
}

void Script::OnCellChange(NetworkID id, unsigned int cell)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnCellChange)
				script->fOnCellChange(id, cell);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnCellChange"))
			PAWN::Call(script->amx, "OnCellChange", "il", 0, cell, id);
	}
}

void Script::OnLockChange(NetworkID id, NetworkID player, unsigned int lock)
{
	if (lock < 5 && DB::Terminal::Lookup(GameFactory::GetObject(id)->GetBase()))
		lock *= 25;

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnLockChange)
				script->fOnLockChange(id, player, lock);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnLockChange"))
			PAWN::Call(script->amx, "OnLockChange", "ill", 0, lock, player, id);
	}
}

void Script::OnContainerItemChange(NetworkID id, unsigned int baseID, signed int count, double condition)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnContainerItemChange)
				script->fOnContainerItemChange(id, baseID, count, condition);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnContainerItemChange"))
			PAWN::Call(script->amx, "OnContainerItemChange", "fqil", 0, condition, count, baseID, id);
	}
}

void Script::OnActorValueChange(NetworkID id, unsigned char index, bool base, double value)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (base)
			{
				if (script->fOnActorBaseValueChange)
					script->fOnActorBaseValueChange(id, index, value);
			}
			else if (script->fOnActorValueChange)
				script->fOnActorValueChange(id, index, value);
		}
		else
		{
			if (base)
			{
				if (PAWN::IsCallbackPresent(script->amx, "OnActorBaseValueChange"))
					PAWN::Call(script->amx, "OnActorBaseValueChange", "fil", 0, value, (unsigned int) index, id);
			}
			else if (PAWN::IsCallbackPresent(script->amx, "OnActorValueChange"))
				PAWN::Call(script->amx, "OnActorValueChange", "fil", 0, value, (unsigned int) index, id);
		}
	}
}

void Script::OnActorAlert(NetworkID id, bool alerted)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorAlert)
				script->fOnActorAlert(id, alerted);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorAlert"))
			PAWN::Call(script->amx, "OnActorAlert", "il", 0, (unsigned int) alerted, id);
	}
}

void Script::OnActorSneak(NetworkID id, bool sneaking)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorSneak)
				script->fOnActorSneak(id, sneaking);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorSneak"))
			PAWN::Call(script->amx, "OnActorSneak", "il", 0, (unsigned int) sneaking, id);
	}
}

void Script::OnActorDeath(NetworkID id, NetworkID killer, unsigned short limbs, signed char cause)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorDeath)
				script->fOnActorDeath(id, killer, limbs, cause);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorDeath"))
			PAWN::Call(script->amx, "OnActorDeath", "qill", 0, cause, limbs, killer, id);
	}
}

void Script::OnActorEquipItem(NetworkID id, unsigned int baseID, double condition)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorEquipItem)
				script->fOnActorEquipItem(id, baseID, condition);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorEquipItem"))
			PAWN::Call(script->amx, "OnActorEquipItem", "fil", 0, condition, baseID, id);
	}
}

void Script::OnActorUnequipItem(NetworkID id, unsigned int baseID, double condition)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorUnequipItem)
				script->fOnActorUnequipItem(id, baseID, condition);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorUnequipItem"))
			PAWN::Call(script->amx, "OnActorUnequipItem", "fil", 0, condition, baseID, id);
	}
}

void Script::OnActorDropItem(NetworkID id, unsigned int baseID, unsigned int count, double condition)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorDropItem)
				script->fOnActorDropItem(id, baseID, count, condition);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorDropItem"))
			PAWN::Call(script->amx, "OnActorDropItem", "fiil", 0, condition, count, baseID, id);
	}
}

void Script::OnActorPickupItem(NetworkID id, unsigned int baseID, unsigned int count, double condition, unsigned int owner)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorPickupItem)
				script->fOnActorPickupItem(id, baseID, count, condition, owner);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorPickupItem"))
			PAWN::Call(script->amx, "OnActorPickupItem", "ifiil", 0, owner, condition, count, baseID, id);
	}
}

void Script::OnActorPunch(NetworkID id, bool power)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorPunch)
				script->fOnActorPunch(id, power);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorPunch"))
			PAWN::Call(script->amx, "OnActorPunch", "il", 0, power, id);
	}
}

void Script::OnActorFireWeapon(NetworkID id, unsigned int weapon)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorFireWeapon)
				script->fOnActorFireWeapon(id, weapon);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnActorFireWeapon"))
			PAWN::Call(script->amx, "OnActorFireWeapon", "il", 0, weapon, id);
	}
}

void Script::OnPlayerDisconnect(NetworkID id, Reason reason)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnPlayerDisconnect)
				script->fOnPlayerDisconnect(id, reason);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnPlayerDisconnect"))
			PAWN::Call(script->amx, "OnPlayerDisconnect", "il", 0, (unsigned int) reason, id);
	}
}

unsigned int Script::OnPlayerRequestGame(NetworkID id)
{
	unsigned int result = 0;

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnPlayerRequestGame)
				result = script->fOnPlayerRequestGame(id);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnPlayerRequestGame"))
			result = (unsigned int) PAWN::Call(script->amx, "OnPlayerRequestGame", "l", 0, id);
	}

	return result;
}

bool Script::OnPlayerChat(NetworkID id, string& message)
{
	bool result = true;

	char _message[MAX_CHAT_LENGTH + 1];
	ZeroMemory(_message, sizeof(_message));
	strncpy(_message, message.c_str(), sizeof(_message) - 1);

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnPlayerChat)
				result = script->fOnPlayerChat(id, _message);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnPlayerChat"))
			result = static_cast<bool>(PAWN::Call(script->amx, "OnPlayerChat", "sl", 1, _message, id));
	}

	message.assign(_message);

	return result;
}

bool Script::OnClientAuthenticate(const string& name, const string& pwd)
{
	bool result = true;

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnClientAuthenticate)
				result = script->fOnClientAuthenticate(name.c_str(), pwd.c_str());
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnClientAuthenticate"))
			result = static_cast<bool>(PAWN::Call(script->amx, "OnClientAuthenticate", "ss", 0, pwd.c_str(), name.c_str()));
	}

	return result;
}

void Script::OnGameYearChange(unsigned int year)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnGameYearChange)
				script->fOnGameYearChange(year);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnGameYearChange"))
			PAWN::Call(script->amx, "OnGameYearChange", "i", 0, year);
	}
}

void Script::OnGameMonthChange(unsigned int month)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnGameMonthChange)
				script->fOnGameMonthChange(month);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnGameMonthChange"))
			PAWN::Call(script->amx, "OnGameMonthChange", "i", 0, month);
	}
}

void Script::OnGameDayChange(unsigned int day)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnGameDayChange)
				script->fOnGameDayChange(day);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnGameDayChange"))
			PAWN::Call(script->amx, "OnGameDayChange", "i", 0, day);
	}
}

void Script::OnGameHourChange(unsigned int hour)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnGameHourChange)
				script->fOnGameHourChange(hour);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnGameHourChange"))
			PAWN::Call(script->amx, "OnGameHourChange", "i", 0, hour);
	}
}

void Script::OnServerInit()
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnServerInit)
				script->fOnServerInit();
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnServerInit"))
			PAWN::Call(script->amx, "OnServerInit", "", 0);
	}
}

void Script::OnServerExit()
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnServerExit)
				script->fOnServerExit();
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnServerExit"))
			PAWN::Call(script->amx, "OnServerExit", "", 0);
	}
}

const char* Script::ValueToString(unsigned char index)
{
	static string value;
	value.assign(API::RetrieveValue_Reverse(index));
	return value.c_str();
}

const char* Script::AxisToString(unsigned char index)
{
	static string axis;
	axis.assign(API::RetrieveAxis_Reverse(index));
	return axis.c_str();
}

const char* Script::AnimToString(unsigned char index)
{
	static string anim;
	anim.assign(API::RetrieveAnim_Reverse(index));
	return anim.c_str();
}

const char* Script::BaseToString(unsigned int baseID)
{
	static string base;
	base.clear();

	auto record = DB::Record::Lookup(baseID);

	if (record)
		base.assign(record->GetName());

	return base.c_str();
}

bool Script::Kick(NetworkID id)
{
	if (!GameFactory::GetObject<Player>(id))
		return false;

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_END>(Reason::ID_REASON_KICK),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
	});

	return true;
}

bool Script::UIMessage(NetworkID id, const char* message, unsigned char emoticon)
{
	string _message(message);

	if (_message.length() > MAX_MESSAGE_LENGTH)
		_message.resize(MAX_MESSAGE_LENGTH);

	if (id)
	{
		if (!GameFactory::GetObject<Player>(id))
			return false;
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_MESSAGE>(_message, emoticon),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(nullptr))
	});

	return true;
}

bool Script::ChatMessage(NetworkID id, const char* message)
{
	string _message(message);

	if (_message.length() > MAX_CHAT_LENGTH)
		_message.resize(MAX_CHAT_LENGTH);

	if (id)
	{
		if (!GameFactory::GetObject<Player>(id))
			return false;
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_CHAT>(_message),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(nullptr))
	});

	return true;
}

void Script::SetSpawnCell(unsigned int cell)
{
	try
	{
		Player::SetSpawnCell(cell);
	}
	catch (...) {}
}

void Script::SetGameWeather(unsigned int weather)
{
	if (Script::gameWeather == weather)
		return;

	if (DB::Record::IsValidWeather(weather))
	{
		Script::gameWeather = weather;

		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_WEATHER>(weather),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}
}

void Script::SetGameTime(signed long long time)
{
	Time64_T t_new = time;

	TM _tm_new;
	auto success = gmtime64_r(&t_new, &_tm_new);

	if (!success)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (_tm.tm_year != _tm_new.tm_year)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, _tm_new.tm_year),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}

	if (_tm.tm_mon != _tm_new.tm_mon)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, _tm_new.tm_mon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}

	if (_tm.tm_mday != _tm_new.tm_mday)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, _tm_new.tm_mday),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}

	if (_tm.tm_hour != _tm_new.tm_hour)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, _tm_new.tm_hour),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}

	gameTime.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t_new));
}

void Script::SetGameYear(unsigned int year)
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (_tm.tm_year != year)
	{
		_tm.tm_year = year - 1900;
		t = mktime64(&_tm);

		if (t != -1)
		{
			gameTime.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, year),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});
		}
	}
}

void Script::SetGameMonth(unsigned int month)
{
	if (month > 11)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (_tm.tm_mon != month)
	{
		_tm.tm_mon = month;
		t = mktime64(&_tm);

		if (t != -1)
		{
			gameTime.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, month),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});
		}
	}
}

void Script::SetGameDay(unsigned int day)
{
	if (!day || day > 31)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (_tm.tm_mday != day)
	{
		_tm.tm_mday = day;
		t = mktime64(&_tm);

		if (t != -1)
		{
			gameTime.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, day),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});
		}
	}
}

void Script::SetGameHour(unsigned int hour)
{
	if (hour > 23)
		return;

	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();

	TM _tm;
	gmtime64_r(&t, &_tm);

	if (_tm.tm_hour != hour)
	{
		_tm.tm_hour = hour;
		t = mktime64(&_tm);

		if (t != -1)
		{
			gameTime.first = chrono::time_point<chrono::system_clock>(chrono::seconds(t));

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, hour),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});
		}
	}
}

void Script::SetTimeScale(double scale)
{
	gameTime.second = scale;
}

bool Script::IsValid(NetworkID id)
{
	return GameFactory::GetType(id) || scriptIL.count(id);
}

bool Script::IsObject(NetworkID id)
{
	return (GameFactory::GetType(id) & ALL_OBJECTS);
}

bool Script::IsItem(NetworkID id)
{
	return (GameFactory::GetType(id) & ID_ITEM);
}

bool Script::IsContainer(NetworkID id)
{
	return (GameFactory::GetType(id) & ALL_CONTAINERS);
}

bool Script::IsActor(NetworkID id)
{
	return (GameFactory::GetType(id) & ALL_ACTORS);
}

bool Script::IsPlayer(NetworkID id)
{
	return (GameFactory::GetType(id) & ID_PLAYER);
}

bool Script::IsInterior(unsigned int cell)
{
	return !DB::Exterior::Lookup(cell) && DB::Record::Lookup(cell, "CELL");
}

bool Script::IsItemList(NetworkID id)
{
	return scriptIL.count(id);
}

unsigned int Script::GetConnection(NetworkID id)
{
	unsigned int value = UINT_MAX;
	Client* client = Client::GetClientFromPlayer(id);

	if (client)
		value = client->GetID();

	return value;
}

unsigned int Script::GetList(unsigned char type, NetworkID** data)
{
	static vector<NetworkID> _data;
	_data = GameFactory::GetIDObjectTypes(type);
	*data = &_data[0];
	return _data.size();
}

void Script::GetChatboxPos(double* X, double* Y)
{
	auto pos = Player::GetChatboxPos();
	*X = pos.first;
	*Y = pos.second;
}

void Script::GetChatboxSize(double* X, double* Y)
{
	auto size = Player::GetChatboxSize();
	*X = size.first;
	*Y = size.second;
}

unsigned int Script::GetGameWeather()
{
	return Script::gameWeather;
}

signed long long Script::GetGameTime()
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();
	return t;
}

unsigned int Script::GetGameYear()
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_year + 1900;
}

unsigned int Script::GetGameMonth()
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_mon;
}

unsigned int Script::GetGameDay()
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_mday;
}

unsigned int Script::GetGameHour()
{
	Time64_T t = chrono::duration_cast<chrono::seconds>(gameTime.first.time_since_epoch()).count();
	return gmtime64(&t)->tm_hour;
}

double Script::GetTimeScale()
{
	return gameTime.second;
}

NetworkID Script::GetID(unsigned int refID)
{
	auto object = GameFactory::GetObject(refID);

	if (object)
		return object->GetNetworkID();

	return 0;
}

unsigned int Script::GetReference(NetworkID id)
{
	auto object = GameFactory::GetObject(id);

	if (object)
		return object->GetReference();

	return 0;
}

unsigned int Script::GetBase(NetworkID id)
{
	auto object = GameFactory::GetObject(id);

	if (object)
		return object->GetBase();

	return 0;
}

void Script::GetPos(NetworkID id, double* X, double* Y, double* Z)
{
	*X = 0.00;
	*Y = 0.00;
	*Z = 0.00;

	auto object = GameFactory::GetObject(id);

	if (object)
	{
		*X = object->GetNetworkPos(Axis_X);
		*Y = object->GetNetworkPos(Axis_Y);
		*Z = object->GetNetworkPos(Axis_Z);
	}
}

void Script::GetAngle(NetworkID id, double* X, double* Y, double* Z)
{
	*X = 0.00;
	*Y = 0.00;
	*Z = 0.00;

	auto object = GameFactory::GetObject(id);

	if (object)
	{
		*X = object->GetAngle(Axis_X);
		*Y = object->GetAngle(Axis_Y);
		*Z = object->GetAngle(Axis_Z);
	}
}

unsigned int Script::GetCell(NetworkID id)
{
	auto object = GameFactory::GetObject(id);

	if (object)
		return object->GetNetworkCell();

	return 0;
}

unsigned int Script::GetLock(NetworkID id)
{
	auto object = GameFactory::GetObject(id);

	if (object)
		return object->GetLockLevel();

	return 0;
}

unsigned int Script::GetOwner(NetworkID id)
{
	auto object = GameFactory::GetObject(id);

	if (object)
		return object->GetOwner();

	return 0;
}

const char* Script::GetBaseName(NetworkID id)
{
	static string name;
	auto object = GameFactory::GetObject(id);

	if (object)
	{
		name.assign(object->GetName());
		return name.c_str();
	}

	return "";
}

bool Script::IsNearPoint(NetworkID id, double X, double Y, double Z, double R)
{
	auto object = GameFactory::GetObject(id);

	if (object)
		return object->IsNearPoint(X, Y, Z, R);

	return false;
}

NetworkID Script::GetItemContainer(NetworkID id)
{
	auto item = GameFactory::GetObject<Item>(id);

	if (item)
		return item->GetItemContainer();

	return 0;
}

unsigned int Script::GetItemCount(NetworkID id)
{
	auto item = GameFactory::GetObject<Item>(id);

	if (item)
		return item->GetItemCount();

	return 0;
}

double Script::GetItemCondition(NetworkID id)
{
	auto item = GameFactory::GetObject<Item>(id);

	if (item)
		return item->GetItemCondition();

	return 0.00;
}

bool Script::GetItemEquipped(NetworkID id)
{
	auto item = GameFactory::GetObject<Item>(id);

	if (item)
		return item->GetItemEquipped();

	return false;
}

bool Script::GetItemSilent(NetworkID id)
{
	auto item = GameFactory::GetObject<Item>(id);

	if (item)
		return item->GetItemSilent();

	return false;
}

bool Script::GetItemStick(NetworkID id)
{
	auto item = GameFactory::GetObject<Item>(id);

	if (item)
		return item->GetItemStick();

	return false;
}

unsigned int Script::GetContainerItemCount(NetworkID id, unsigned int baseID)
{
	auto container = GameFactory::GetObject<Container>(id);

	if (container || scriptIL.count(id))
	{
		ItemList* IL = container ? &container->IL : scriptIL[id].get();
		return IL->GetItemCount(baseID);
	}

	return 0;
}

unsigned int Script::GetContainerItemList(NetworkID id, NetworkID** data)
{
	static vector<NetworkID> _data;
	*data = nullptr;

	auto container = GameFactory::GetObject<Container>(id);

	if (container || scriptIL.count(id))
	{
		ItemList* IL = container ? &container->IL : scriptIL[id].get();
		_data.assign(IL->GetItemList().begin(), IL->GetItemList().end());
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	}

	return 0;
}

double Script::GetActorValue(NetworkID id, unsigned char index)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	try
	{
		if (actor)
			return actor->GetActorValue(index);
	}
	catch (...) {}

	return 0.00;
}

double Script::GetActorBaseValue(NetworkID id, unsigned char index)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	try
	{
		if (actor)
			return actor->GetActorBaseValue(index);
	}
	catch (...) {}

	return 0.00;
}

unsigned int Script::GetActorIdleAnimation(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorIdleAnimation();

	return 0x00000000;
}

unsigned char Script::GetActorMovingAnimation(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorMovingAnimation();

	return 0x00;
}

unsigned char Script::GetActorWeaponAnimation(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorWeaponAnimation();

	return 0x00;
}

bool Script::GetActorAlerted(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorAlerted();

	return false;
}

bool Script::GetActorSneaking(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorSneaking();

	return false;
}

bool Script::GetActorDead(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorDead();

	return false;
}

unsigned int Script::GetActorBaseRace(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorRace();

	return 0x00000000;
}

bool Script::GetActorBaseSex(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->GetActorFemale();

	return false;
}

bool Script::IsActorJumping(NetworkID id)
{
	auto actor = GameFactory::GetObject<Actor>(id);

	if (actor)
		return actor->IsActorJumping();

	return false;
}

unsigned int Script::GetPlayerRespawnTime(NetworkID id)
{
	auto player = GameFactory::GetObject<Player>(id);

	if (player)
		return player->GetPlayerRespawnTime();

	return 0;
}

unsigned int Script::GetPlayerSpawnCell(NetworkID id)
{
	auto player = GameFactory::GetObject<Player>(id);

	if (player)
		return player->GetPlayerSpawnCell();

	return 0x00000000;
}

bool Script::GetPlayerConsoleEnabled(NetworkID id)
{
	auto player = GameFactory::GetObject<Player>(id);

	if (player)
		return player->GetPlayerConsoleEnabled();

	return false;
}

bool Script::GetPlayerChatboxEnabled(NetworkID id)
{
	auto player = GameFactory::GetObject<Player>(id);

	if (player)
		return player->GetPlayerChatboxEnabled();

	return false;
}

bool Script::GetPlayerChatboxLocked(NetworkID id)
{
	auto player = GameFactory::GetObject<Player>(id);

	if (player)
		return player->GetPlayerChatboxLocked();

	return false;
}

void Script::GetPlayerChatboxPos(NetworkID id, double* X, double* Y)
{
	auto player = GameFactory::GetObject<Player>(id);

	if (player)
	{
		auto pos = player->GetPlayerChatboxPos();
		*X = pos.first;
		*Y = pos.second;
	}
}

void Script::GetPlayerChatboxSize(NetworkID id, double* X, double* Y)
{
	auto player = GameFactory::GetObject<Player>(id);

	if (player)
	{
		auto size = player->GetPlayerChatboxSize();
		*X = size.first;
		*Y = size.second;
	}
}

NetworkID Script::CreateObject(unsigned int baseID, NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	NetworkID result = 0;
	FactoryObject<Object> object;
	FactoryObject<Object> _object;

	if (id && !GameFactory::GetType(id))
		return result;

	try
	{
		auto reference = GameFactory::GetMultiple<Object>(vector<NetworkID>{id, GameFactory::CreateInstance(ID_OBJECT, baseID)});

		if (id)
			object = reference[0].get();

		_object = vaultcast<Object>(reference[1]).get();
	}
	catch (...) { return result; }

	result = _object->GetNetworkID();

	SetupObject(_object, object, cell, X, Y, Z);

	Network::Queue(NetworkResponse{Network::CreateResponse(
		_object->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	return result;
}

bool Script::DestroyObject(NetworkID id)
{
	bool state = false;

	auto reference = GameFactory::GetObject(id);

	if (!reference)
		return scriptIL.erase(id) != 0;
	else if (vaultcast<Player>(reference))
		return false;

	auto& object = reference.get();

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	GameFactory::DestroyInstance(object);

	return state;
}

bool Script::SetPos(NetworkID id, double X, double Y, double Z)
{
	bool state = false;

	auto reference = GameFactory::GetObject(id);

	if (!reference || reference->IsPersistent())
		return state;

	auto& object = reference.get();

	unsigned int cell = object->GetNetworkCell();
	const DB::Exterior* new_cell = nullptr;

	auto exterior = DB::Exterior::Lookup(cell);

	if (exterior)
	{
		exterior = DB::Exterior::Lookup(exterior->GetWorld(), X, Y);

		if (!exterior)
			return state;

		new_cell = *exterior;
	} // interior, can't check pos (yet? which are the bounds of interiors?)

	NetworkResponse response;
	unsigned int _new_cell = 0x00000000;

	if (static_cast<bool>(object->SetNetworkPos(Axis_X, X)) | static_cast<bool>(object->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(object->SetNetworkPos(Axis_Z, Z)))
	{
		object->SetGamePos(Axis_X, X);
		object->SetGamePos(Axis_Y, Y);
		object->SetGamePos(Axis_Z, Z);

		if (new_cell)
		{
			_new_cell = new_cell->GetBase();

			if (object->SetNetworkCell(_new_cell))
			{
				object->SetGameCell(_new_cell);

				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, _new_cell),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				);

				auto player = vaultcast<Player>(object);

				if (player)
				{
					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_cell->GetWorld(), new_cell->GetX(), new_cell->GetY(), false),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
					);

					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(reference->GetNetworkID(), player->GetPlayerCellContext()),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
					);
				}
			}
			else
				_new_cell = 0x00000000;
		}

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_POS>(id, X, Y, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		);

		Network::Queue(move(response));

		state = true;
	}

	return state;
}

bool Script::SetAngle(NetworkID id, double X, double Y, double Z)
{
	bool state = false;

	auto reference = GameFactory::GetObject(id);

	if (!reference)
		return state;

	auto& object = reference.get();

	if (object->SetAngle(Axis_X, X))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(id, Axis_X, X),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		state = true;
	}

	if (object->SetAngle(Axis_Y, Y))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(id, Axis_Y, Y),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		state = true;
	}

	if (object->SetAngle(Axis_Z, Z))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(id, Axis_Z, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		state = true;
	}

	return state;
}

bool Script::SetCell(NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	return SetCell_(id, cell, X, Y, Z, false);
}

bool Script::SetCell_(NetworkID id, unsigned int cell, double X, double Y, double Z, bool nosend)
{
	bool state = false;

	auto reference = GameFactory::GetObject(id);

	if (!reference || reference->IsPersistent())
		return state;

	auto& object = reference.get();

	bool update_pos = X != 0.00 && Y != 0.00 && Z != 0.00;
	const DB::Record* new_interior = nullptr;
	const DB::Exterior* new_exterior = nullptr;

	auto exterior = DB::Exterior::Lookup(cell);

	if (!exterior)
	{
		auto interior = DB::Record::Lookup(cell, "CELL");

		if (!interior)
			return state;

		new_interior = *interior;
	}
	else if (update_pos)
	{
		exterior = DB::Exterior::Lookup(exterior->GetWorld(), X, Y);

		if (!exterior || (new_exterior = *exterior)->GetBase() != cell)
			return state;
	}
	else
		new_exterior = *exterior;

	NetworkResponse response;

	auto player = vaultcast<Player>(object);

	if (object->SetNetworkCell(cell))
	{
		object->SetGameCell(cell);

		if (!nosend)
		{
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			);

			if (player)
			{
				if (new_interior)
					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(id, new_interior->GetName(), false),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
					);
				else
					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_exterior->GetWorld(), new_exterior->GetX(), new_exterior->GetY(), false),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
					);

				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(reference->GetNetworkID(), player->GetPlayerCellContext()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
				);
			}
		}

		state = true;
	}
	else
		cell = 0x00000000;

	if (update_pos && (static_cast<bool>(object->SetNetworkPos(Axis_X, X)) | static_cast<bool>(object->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(object->SetNetworkPos(Axis_Z, Z))))
	{
		object->SetGamePos(Axis_X, X);
		object->SetGamePos(Axis_Y, Y);
		object->SetGamePos(Axis_Z, Z);

		if (!nosend)
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(id, X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			);

		state = true;
	}

	if (state && !nosend)
		Network::Queue(move(response));

	return state;
}

bool Script::SetLock(NetworkID id, unsigned int lock)
{
	bool state = false;

	auto reference = GameFactory::GetObject(id);

	if (!reference)
		return state;

	auto& object = reference.get();

	if (object->GetLockLevel() == UINT_MAX - 1)
		return state;

	if (lock != UINT_MAX)
	{
		lock = ceil(static_cast<double>(lock) / 25.0) * 25;

		if (lock > 100)
			lock = 255;

		if (DB::Terminal::Lookup(object->GetBase()))
		{
			if (lock == 255)
				lock = 5;
			else
				lock /= 25;
		}
	}

	if (object->SetLockLevel(lock))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_LOCK>(id, lock),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		state = true;
	}

	return state;
}

bool Script::SetOwner(NetworkID id, unsigned int owner)
{
	bool state = false;

	auto reference = GameFactory::GetObject(id);

	if (!reference || owner == PLAYER_BASE)
		return state;

	auto& object = reference.get();
	auto npc = DB::NPC::Lookup(owner);

	if (npc && object->SetOwner(owner))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_OWNER>(id, owner),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		state = true;
	}

	return state;
}

bool Script::SetBaseName(NetworkID id, const char* name)
{
	if (!name)
		return false;

	string _name(name);

	if (_name.length() > MAX_PLAYER_NAME)
		return false;

	vector<FactoryObject<Object>> reference = GameFactory::GetObjectTypes<Object>(ALL_OBJECTS);
	auto it = find_if(reference.begin(), reference.end(), [&](const FactoryObject<Object>& object) { return object->GetNetworkID() == id; });

	if (it == reference.end())
		return false;

	unsigned int baseID = (*it)->GetBase();

	if (baseID == PLAYER_BASE)
		return false;

	DB::Record::Lookup(baseID)->SetDescription(_name);

	for (const auto& object : reference)
	{
		auto item = vaultcast<Item>(object);

		if (item && item->GetItemContainer())
			continue;

		if (object->GetBase() == baseID)
		{
			if (object->SetName(_name))
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_NAME>(object->GetNetworkID(), _name),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});
		}
	}

	return true;
}

NetworkID Script::CreateItem(unsigned int baseID, NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	NetworkID result = 0;
	FactoryObject<Object> object;
	FactoryObject<Item> item;

	if (id && !GameFactory::GetType(id))
		return result;

	try
	{
		auto reference = GameFactory::GetMultiple<Object>(vector<NetworkID>{id, GameFactory::CreateInstance(ID_ITEM, baseID)});

		if (id)
			object = reference[0].get();

		item = vaultcast<Item>(reference[1]).get();
	}
	catch (...) { return result; }

	result = item->GetNetworkID();

	SetupItem(item, object, cell, X, Y, Z);

	Network::Queue(NetworkResponse{Network::CreateResponse(
		item->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	return result;
}

bool Script::SetItemCount(NetworkID id, unsigned int count)
{
	if (!count)
		return false;

	auto reference = GameFactory::GetObject<Item>(id);

	if (!reference)
		return false;

	auto& item = reference.get();

	if (!item->GetItemContainer() && item->SetItemCount(count))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_COUNT>(item->GetNetworkID(), count),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	}

	return false;
}

bool Script::SetItemCondition(NetworkID id, double condition)
{
	if (condition < 0.00 || condition > 100.0)
		return false;

	auto reference = GameFactory::GetObject<Item>(id);

	if (!reference)
		return false;

	auto& item = reference.get();

	auto _item = DB::Item::Lookup(item->GetBase());

	if (!_item)
		return false;

	if (!item->GetItemContainer() && item->SetItemCondition(condition))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONDITION>(item->GetNetworkID(), condition, static_cast<unsigned int>(_item->GetHealth() * (condition / 100.0))),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	}

	return false;
}

NetworkID Script::CreateContainer(unsigned int baseID, NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	NetworkID result = 0;
	FactoryObject<Object> object;
	FactoryObject<Container> container;

	if (id && !GameFactory::GetType(id))
		return result;

	try
	{
		auto reference = GameFactory::GetMultiple<Object>(vector<NetworkID>{id, GameFactory::CreateInstance(ID_CONTAINER, baseID)});

		if (id)
			object = reference[0].get();

		container = vaultcast<Container>(reference[1]).get();
	}
	catch (...) { return result; }

	result = container->GetNetworkID();

	SetupContainer(container, object, cell, X, Y, Z);

	Network::Queue(NetworkResponse{Network::CreateResponse(
		container->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	return result;
}

NetworkID Script::CreateItemList(NetworkID source, unsigned int baseID)
{
	// make_unique
	auto IL = unique_ptr<ItemList>(new ItemList(0));
	NetworkID id = IL->GetNetworkID();
	scriptIL.emplace(id, move(IL));

	if (source || baseID)
		AddItemList(id, source, baseID);

	return id;
}

bool Script::AddItem(NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent)
{
	try
	{
		if (count)
		{
			auto reference = GameFactory::GetObject<Container>(id);

			if (!reference && !scriptIL.count(id))
				return false;

			ItemList* IL = reference ? &reference->IL : scriptIL[id].get();
			auto diff = IL->AddItem(baseID, count, condition, silent);

			if (reference)
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, ItemList::ToNetDiff(diff), ItemList::NetDiff()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});

			IL->ApplyDiff(diff);

			return true;
		}
	}
	catch (...) {} // invalid item baseID

	return false;
}

void Script::AddItemList(NetworkID id, NetworkID source, unsigned int baseID)
{
	auto reference = GameFactory::GetObject<Container>(id);

	if (!reference && !scriptIL.count(id))
		return;

	if (source)
	{
		if (reference)
			GameFactory::LeaveReference(reference.get());

		reference = GameFactory::GetObject<Container>(source);

		if (!reference && !scriptIL.count(source))
			return;

		ItemList* IL = reference ? &reference->IL : scriptIL[source].get();
		auto items = GameFactory::GetMultiple<Item>(vector<NetworkID>{IL->GetItemList().begin(), IL->GetItemList().end()});

		for (auto& item : items)
		{
			AddItem(id, item->GetBase(), item->GetItemCount(), item->GetItemCondition(), item->GetItemSilent());

			if (item->GetItemEquipped())
				EquipItem(id, item->GetBase(), item->GetItemSilent(), item->GetItemStick());
		}
	}
	else if (reference || baseID)
	{
		const auto& items = DB::BaseContainer::Lookup(reference ? reference->GetBase() : baseID);

		for (const auto* item : items)
		{
			if (item->GetItem() & 0xFF000000)
				continue;

			AddItem(id, item->GetItem(), item->GetCount(), item->GetCondition(), true);
		}
	}
}

unsigned int Script::RemoveItem(NetworkID id, unsigned int baseID, unsigned int count, bool silent)
{
	unsigned int removed = 0;

	if (count)
	{
		auto reference = GameFactory::GetObject<Container>(id);

		if (!reference && !scriptIL.count(id))
			return removed;

		ItemList* IL = reference ? &reference->IL : scriptIL[id].get();
		auto diff = IL->RemoveItem(baseID, count, silent);

		if (!diff.first.empty() || !diff.second.empty())
		{
			if (reference)
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, ItemList::ToNetDiff(diff), ItemList::NetDiff()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});

			auto gamediff = IL->ApplyDiff(diff);
			removed = abs(gamediff.front().second.count);
		}
	}

	return removed;
}

void Script::RemoveAllItems(NetworkID id)
{
	auto reference = GameFactory::GetObject<Container>(id);

	if (!reference && !scriptIL.count(id))
		return;

	ItemList* IL = reference ? &reference->IL : scriptIL[id].get();
	auto diff = IL->RemoveAllItems();

	if (!diff.first.empty() || !diff.second.empty())
	{
		if (reference)
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, ItemList::ToNetDiff(diff), ItemList::NetDiff()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

		IL->ApplyDiff(diff);
	}
}

NetworkID Script::CreateActor(unsigned int baseID, NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	NetworkID result = 0;
	FactoryObject<Object> object;
	FactoryObject<Actor> actor;

	if (id && !GameFactory::GetType(id))
		return result;

	try
	{
		auto reference = GameFactory::GetMultiple<Object>(vector<NetworkID>{id, GameFactory::CreateInstance(ID_ACTOR, baseID)});

		if (id)
			object = reference[0].get();

		actor = vaultcast<Actor>(reference[1]).get();
	}
	catch (...) { return result; }

	result = actor->GetNetworkID();

	SetupActor(actor, object, cell, X, Y, Z);

	Network::Queue(NetworkResponse{Network::CreateResponse(
		actor->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	return result;
}

void Script::SetActorValue(NetworkID id, unsigned char index, double value)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference)
		return;

	auto& actor = reference.get();

	try
	{
		if (actor->SetActorValue(index, value))
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(actor->GetNetworkID(), false, index, value),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});
	}
	catch (...) {}
}

void Script::SetActorBaseValue(NetworkID id, unsigned char index, double value)
{
	vector<FactoryObject<Actor>> reference = GameFactory::GetObjectTypes<Actor>(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [&](const FactoryObject<Actor>& actor) { return actor->GetNetworkID() == id; });

	if (it == reference.end())
		return;

	unsigned int baseID = (*it)->GetBase();

	if (baseID == PLAYER_BASE)
		return;

	for (const auto& actor : reference)
	{
		if (actor->GetBase() == baseID)
		{
			try
			{
				if (actor->SetActorBaseValue(index, value))
					Network::Queue(NetworkResponse{Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(actor->GetNetworkID(), true, index, value),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});
			}
			catch (...) {}
		}
	}
}

bool Script::EquipItem(NetworkID id, unsigned int baseID, bool silent, bool stick)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference && !scriptIL.count(id))
		return false;

	ItemList* IL = reference ? &reference->IL : scriptIL[id].get();
	auto diff = IL->EquipItem(baseID, silent, stick);

	if (!diff.first.empty() || !diff.second.empty())
	{
		if (reference)
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, ItemList::ToNetDiff(diff), ItemList::NetDiff()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

		IL->ApplyDiff(diff);

		return true;
	}

	return false;
}

bool Script::UnequipItem(NetworkID id, unsigned int baseID, bool silent, bool stick)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference && !scriptIL.count(id))
		return false;

	ItemList* IL = reference ? &reference->IL : scriptIL[id].get();
	auto diff = IL->UnequipItem(baseID, silent, stick);

	if (!diff.first.empty() || !diff.second.empty())
	{
		if (reference)
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, ItemList::ToNetDiff(diff), ItemList::NetDiff()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

		IL->ApplyDiff(diff);

		return true;
	}

	return false;
}

bool Script::SetActorMovingAnimation(NetworkID id, unsigned char anim)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference || vaultcast<Player>(reference))
		return false;

	auto& actor = reference.get();

	try
	{
		if (actor->SetActorMovingAnimation(anim))
		{
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), actor->GetActorIdleAnimation(), anim, actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), actor->GetActorAlerted(), actor->GetActorSneaking(), false),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			return true;
		}
	}
	catch (...) {}

	return false;
}

bool Script::SetActorWeaponAnimation(NetworkID id, unsigned char anim)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference || vaultcast<Player>(reference))
		return false;

	auto& actor = reference.get();

	try
	{
		if (actor->SetActorWeaponAnimation(anim))
		{
			bool punching = actor->IsActorPunching();
			bool power_punching = actor->IsActorPowerPunching();
			bool firing = actor->IsActorFiring();

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), anim, actor->GetActorAlerted(), actor->GetActorSneaking(), !punching && !power_punching && firing),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			return true;
		}
	}
	catch (...) {}

	return false;
}

bool Script::SetActorAlerted(NetworkID id, bool alerted)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference || vaultcast<Player>(reference))
		return false;

	auto& actor = reference.get();

	if (actor->SetActorAlerted(alerted))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), alerted, actor->GetActorSneaking(), false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	}

	return false;
}

bool Script::SetActorSneaking(NetworkID id, bool sneaking)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference || vaultcast<Player>(reference))
		return false;

	auto& actor = reference.get();

	if (actor->SetActorSneaking(sneaking))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), actor->GetActorAlerted(), sneaking, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	}

	return false;
}

bool Script::FireWeapon(NetworkID id)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference)
		return false;

	auto& actor = reference.get();

	unsigned int baseID = actor->GetEquippedWeapon();
	auto weapon = DB::Weapon::Lookup(baseID);

	if (weapon)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(actor->GetNetworkID(), baseID, weapon->IsAutomatic() ? weapon->GetFireRate() : 0.00),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	}

	return false;
}

bool Script::PlayIdle(NetworkID id, unsigned int idle)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference)
		return false;

	auto& actor = reference.get();

	const DB::Record* record = nullptr;

	auto _record = DB::Record::Lookup(idle, "IDLE");

	if (!_record)
		return false;

	record = *_record;

	if (actor->SetActorIdleAnimation(idle))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_IDLE>(actor->GetNetworkID(), idle, record->GetName()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	}

	return false;
}

void Script::KillActor(NetworkID id, unsigned short limbs, signed char cause)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference)
		return;

	auto& actor = reference.get();

	if (actor->SetActorDead(true))
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(actor->GetNetworkID(), true, limbs, cause),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		auto player = vaultcast<Player>(actor);

		if (player)
			Script::CreateTimerEx(reinterpret_cast<ScriptFunc>(&Script::Timer_Respawn), player->GetPlayerRespawnTime(), "l", player->GetNetworkID());
	}
}

bool Script::SetActorBaseRace(NetworkID id, unsigned int race)
{
	vector<FactoryObject<Actor>> reference = GameFactory::GetObjectTypes<Actor>(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [&](const FactoryObject<Actor>& actor) { return actor->GetNetworkID() == id; });

	if (it == reference.end())
		return false;

	unsigned int baseID = (*it)->GetBase();

	if (DB::Record::Lookup(baseID, "CREA"))
		return false;

	if (baseID == PLAYER_BASE)
		return false;

	if (!DB::Race::Lookup(race))
		return false;

	const DB::NPC* npc = *DB::NPC::Lookup(baseID);
	unsigned int old_race = npc->GetRace();

	if (old_race != race)
	{
		npc->SetRace(race);
		signed int delta_age = DB::Race::Lookup(old_race)->GetAgeDifference(race);
		signed int new_age = DB::Race::Lookup(npc->GetOriginalRace())->GetAgeDifference(race);

		for (const auto& actor : reference)
		{
			if (actor->GetBase() == baseID)
			{
				actor->SetActorRace(race);
				actor->SetActorAge(new_age);

				if (vaultcast<Player>(actor))
					Network::Queue(NetworkResponse{Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_RACE>(actor->GetNetworkID(), race, DB::Race::Lookup(RACE_CAUCASIAN)->GetAgeDifference(race), delta_age),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});
				else
					Network::Queue(NetworkResponse{Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_RACE>(actor->GetNetworkID(), race, new_age, delta_age),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});
			}
		}
	}

	return true;
}

bool Script::AgeActorBaseRace(NetworkID id, signed int age)
{
	auto reference = GameFactory::GetObject<Actor>(id);

	if (!reference)
		return false;

	auto& actor = reference.get();

	if (DB::Record::Lookup(actor->GetBase(), "CREA"))
		return false;

	const DB::Race* race = *DB::Race::Lookup(actor->GetActorRace());
	unsigned int new_race;

	if (age < 0)
	{
		unsigned int _age = abs(age);

		for (unsigned int i = 0; i < _age; ++i)
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

	GameFactory::LeaveReference(actor);

	return SetActorBaseRace(id, new_race);
}

bool Script::SetActorBaseSex(NetworkID id, bool female)
{
	vector<FactoryObject<Actor>> reference = GameFactory::GetObjectTypes<Actor>(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [&](const FactoryObject<Actor>& actor) { return actor->GetNetworkID() == id; });

	if (it == reference.end())
		return false;

	unsigned int baseID = (*it)->GetBase();

	if (DB::Record::Lookup(baseID, "CREA"))
		return false;

	if (baseID == PLAYER_BASE)
		return false;

	const DB::NPC* npc = *DB::NPC::Lookup(baseID);
	bool old_female = npc->IsFemale();

	if (old_female != female)
	{
		npc->SetFemale(female);

		for (const auto& actor : reference)
		{
			if (actor->GetBase() == baseID)
			{
				actor->SetActorFemale(female);

				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_SEX>(actor->GetNetworkID(), female),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});
			}
		}
	}

	return true;
}

void Script::SetPlayerRespawnTime(NetworkID id, unsigned int respawn)
{
	auto reference = GameFactory::GetObject<Player>(id);

	if (reference)
		reference->SetPlayerRespawnTime(respawn);
}

void Script::SetPlayerSpawnCell(NetworkID id, unsigned int cell)
{
	auto reference = GameFactory::GetObject<Player>(id);

	if (!reference)
		return;

	auto& player = reference.get();

	auto _cell = DB::Exterior::Lookup(cell);

	if (!_cell)
	{
		auto record = DB::Record::Lookup(cell, "CELL");

		if (!record)
			return;

		if (player->SetPlayerSpawnCell(cell))
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(player->GetNetworkID(), record->GetName(), true),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
			});
	}
	else if (player->SetPlayerSpawnCell(cell))
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(player->GetNetworkID(), _cell->GetWorld(), _cell->GetX(), _cell->GetY(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
}

void Script::SetPlayerConsoleEnabled(NetworkID id, bool enabled)
{
	auto reference = GameFactory::GetObject<Player>(id);

	if (!reference)
		return;

	auto& player = reference.get();

	if (player->SetPlayerConsoleEnabled(enabled))
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONSOLE>(player->GetNetworkID(), enabled),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
}

void Script::SetPlayerChatboxEnabled(NetworkID id, bool enabled)
{
	auto reference = GameFactory::GetObject<Player>(id);

	if (!reference)
		return;

	auto& player = reference.get();

	if (player->SetPlayerChatboxEnabled(enabled))
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CHAT>(player->GetNetworkID(), player->GetPlayerChatboxEnabled(), player->GetPlayerChatboxLocked(), player->GetPlayerChatboxPos(), player->GetPlayerChatboxSize()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
}

void Script::SetPlayerChatboxLocked(NetworkID id, bool locked)
{
	auto reference = GameFactory::GetObject<Player>(id);

	if (!reference)
		return;

	auto& player = reference.get();

	if (player->SetPlayerChatboxLocked(locked))
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CHAT>(player->GetNetworkID(), player->GetPlayerChatboxEnabled(), player->GetPlayerChatboxLocked(), player->GetPlayerChatboxPos(), player->GetPlayerChatboxSize()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
}

void Script::SetPlayerChatboxPos(NetworkID id, double X, double Y)
{
	auto reference = GameFactory::GetObject<Player>(id);

	if (!reference)
		return;

	auto& player = reference.get();

	if (player->SetPlayerChatboxPos(X, Y))
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CHAT>(player->GetNetworkID(), player->GetPlayerChatboxEnabled(), player->GetPlayerChatboxLocked(), player->GetPlayerChatboxPos(), player->GetPlayerChatboxSize()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
}

void Script::SetPlayerChatboxSize(NetworkID id, double X, double Y)
{
	auto reference = GameFactory::GetObject<Player>(id);

	if (!reference)
		return;

	auto& player = reference.get();

	if (player->SetPlayerChatboxSize(X, Y))
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CHAT>(player->GetNetworkID(), player->GetPlayerChatboxEnabled(), player->GetPlayerChatboxLocked(), player->GetPlayerChatboxPos(), player->GetPlayerChatboxSize()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
}
