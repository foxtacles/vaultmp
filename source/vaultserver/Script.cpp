#include "Script.h"
#include "PAWN.h"
#include "Timer.h"
#include "Public.h"
#include "Client.h"
#include "Network.h"
#include "amx/amxaux.h"
#include "time/time64.h"

#ifdef __WIN32__
#include <winsock2.h>
#else
#include <dlfcn.h>
#endif

using namespace std;
using namespace RakNet;
using namespace Values;

Script::ScriptList Script::scripts;
Script::ScriptItemLists Script::scriptIL;
Script::DeletedObjects Script::deletedStatic;
Script::GameTime Script::gameTime;
Script::GameWeather Script::gameWeather;

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
			GetScript("OnWindowMode", fOnWindowMode);
			GetScript("OnWindowClick", fOnWindowClick);
			GetScript("OnWindowTextChange", fOnWindowTextChange);
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

	deletedStatic.clear();

	gameTime.first = chrono::system_clock::now();
	gameTime.second = 1.0;
	CreateTimer(&Timer_GameTime, 1000);

	gameWeather = DEFAULT_WEATHER;

	auto object_init = [](FactoryObject& object, const DB::Reference* reference)
	{
		const auto& name = DB::Record::Lookup(reference->GetBase())->GetDescription();
		const auto& pos = reference->GetPos();
		const auto& angle = reference->GetAngle();
		auto cell = reference->GetCell();
		auto lock = reference->GetLock();
		object->SetName(name);
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

void Script::SetupObject(FactoryObject& object, FactoryObject& reference, unsigned int cell, double X, double Y, double Z)
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

void Script::SetupItem(FactoryItem& item, FactoryObject& reference, unsigned int cell, double X, double Y, double Z)
{
	SetupObject(item, reference, cell, X, Y, Z);

	item->SetItemCount(1);
	item->SetItemCondition(100.0);
}

void Script::SetupContainer(FactoryContainer& container, FactoryObject& reference, unsigned int cell, double X, double Y, double Z)
{
	SetupObject(container, reference, cell, X, Y, Z);
}

void Script::SetupActor(FactoryActor& actor, FactoryObject& reference, unsigned int cell, double X, double Y, double Z)
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

void Script::SetupWindow(FactoryWindow& window, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text)
{
	window->SetPos(posX, posY, offset_posX, offset_posY);
	window->SetSize(sizeX, sizeY, offset_sizeX, offset_sizeY);
	window->SetVisible(visible);
	window->SetLocked(locked);
	window->SetText(text);
}

void Script::SetupButton(FactoryButton& button, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text)
{
	SetupWindow(button, posX, posY, offset_posX, offset_posY, sizeX, sizeY, offset_sizeX, offset_sizeY, visible, locked, text);
}

void Script::SetupText(FactoryText& text, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text_)
{
	SetupWindow(text, posX, posY, offset_posX, offset_posY, sizeX, sizeY, offset_sizeX, offset_sizeY, visible, locked, text_);
}

void Script::SetupEdit(FactoryEdit& edit, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text)
{
	SetupWindow(edit, posX, posY, offset_posX, offset_posY, sizeX, sizeY, offset_sizeX, offset_sizeY, visible, locked, text);
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

	try
	{
		return Public::Call(name, params);
	}
	catch (...) {}

	return 0;
}

unsigned long long Script::CallPublicPAWN(const char* name, const vector<boost::any>& args)
{
	try
	{
		return Public::Call(name, args);
	}
	catch (...) {}

	return 0;
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

	const auto& values = Player::default_values;

	for (const auto& value : values)
	{
		SetActorBaseValue(id, value.first, value.second.first);
		SetActorValue(id, value.first, value.second.second);
	}

	Network::Queue({Network::CreateResponse(
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
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, _tm_new.tm_year),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		Script::OnGameYearChange(_tm_new.tm_year + 1900);
	}

	if (_tm.tm_mon != _tm_new.tm_mon)
	{
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, _tm_new.tm_mon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		Script::OnGameMonthChange(_tm_new.tm_mon);
	}

	if (_tm.tm_mday != _tm_new.tm_mday)
	{
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, _tm_new.tm_mday),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		Script::OnGameDayChange(_tm_new.tm_mday);
	}

	if (_tm.tm_hour != _tm_new.tm_hour)
	{
		Network::Queue({Network::CreateResponse(
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

void Script::OnWindowMode(NetworkID id, bool enabled)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnWindowMode)
				script->fOnWindowMode(id, enabled);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnWindowMode"))
			PAWN::Call(script->amx, "OnWindowMode", "il", 0, enabled, id);
	}
}

void Script::OnWindowClick(NetworkID id, NetworkID player)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnWindowClick)
				script->fOnWindowClick(id, player);
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnWindowClick"))
			PAWN::Call(script->amx, "OnWindowClick", "ll", 0, player, id);
	}
}

void Script::OnWindowTextChange(NetworkID id, NetworkID player, const string& text)
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnWindowTextChange)
				script->fOnWindowTextChange(id, player, text.c_str());
		}
		else if (PAWN::IsCallbackPresent(script->amx, "OnWindowTextChange"))
			PAWN::Call(script->amx, "OnWindowTextChange", "sll", 0, text.c_str(), player, id);
	}
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
	return GameFactory::Operate<Player, FailPolicy::Bool>(id, [id](FactoryPlayer&) {
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_END>(Reason::ID_REASON_KICK),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
	});
}

bool Script::UIMessage(NetworkID id, const char* message, unsigned char emoticon)
{
	return GameFactory::Operate<Player, FailPolicy::Bool, ObjectPolicy::Expected>(id, [id, message, emoticon](ExpectedPlayer& player) {
		if (id && !player)
			throw VaultException("Invalid parameters: player doesn't exist").stacktrace();

		string message_(message);

		if (message_.length() > MAX_MESSAGE_LENGTH)
			message_.resize(MAX_MESSAGE_LENGTH);

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_MESSAGE>(message_, emoticon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(nullptr))
		});
	});
}

bool Script::ChatMessage(NetworkID id, const char* message)
{
	return GameFactory::Operate<Player, FailPolicy::Bool, ObjectPolicy::Expected>(id, [id, message](ExpectedPlayer& player) {
		if (id && !player)
			throw VaultException("Invalid parameters: player doesn't exist").stacktrace();

		string message_(message);

		if (message_.length() > MAX_MESSAGE_LENGTH)
			message_.resize(MAX_MESSAGE_LENGTH);

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_CHAT>(message_),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(nullptr))
		});
	});
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

		Network::Queue({Network::CreateResponse(
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
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, _tm_new.tm_year),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}

	if (_tm.tm_mon != _tm_new.tm_mon)
	{
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, _tm_new.tm_mon),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}

	if (_tm.tm_mday != _tm_new.tm_mday)
	{
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, _tm_new.tm_mday),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});
	}

	if (_tm.tm_hour != _tm_new.tm_hour)
	{
		Network::Queue({Network::CreateResponse(
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

			Network::Queue({Network::CreateResponse(
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

			Network::Queue({Network::CreateResponse(
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

			Network::Queue({Network::CreateResponse(
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

			Network::Queue({Network::CreateResponse(
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
	return DB::Interior::Lookup(cell).operator bool();
}

bool Script::IsItemList(NetworkID id)
{
	return scriptIL.count(id);
}

bool Script::IsWindow(NetworkID id)
{
	return (GameFactory::GetType(id) & ALL_WINDOWS);
}

bool Script::IsButton(NetworkID id)
{
	return (GameFactory::GetType(id) & ID_BUTTON);
}

bool Script::IsText(NetworkID id)
{
	return (GameFactory::GetType(id) & ID_TEXT);
}

bool Script::IsEdit(NetworkID id)
{
	return (GameFactory::GetType(id) & ID_EDIT);
}

bool Script::IsChatbox(NetworkID id)
{
	return GameFactory::Operate<Window, FailPolicy::Return>(id, [](FactoryWindow& window) {
		return !window->GetLabel().compare(Window::GUI_MAIN_LABEL);
	});
}

unsigned int Script::GetConnection(NetworkID id)
{
	unsigned int value = UINT_MAX;
	Client* client = Client::GetClientFromPlayer(id);

	if (client)
		value = client->GetID();

	return value;
}

unsigned int Script::GetList(unsigned int type, NetworkID** data)
{
	static vector<NetworkID> _data;
	_data = GameFactory::GetIDObjectTypes(type);
	*data = &_data[0];
	return _data.size();
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
	return GameFactory::Operate<Object, FailPolicy::Return>(refID, [](FactoryObject& object) {
		return object->GetNetworkID();
	});
}

unsigned int Script::GetReference(NetworkID id)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [](FactoryObject& object) {
		return object->GetReference();
	});
}

unsigned int Script::GetBase(NetworkID id)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [](FactoryObject& object) {
		return object->GetBase();
	});
}

void Script::GetPos(NetworkID id, double* X, double* Y, double* Z)
{
	*X = 0.00;
	*Y = 0.00;
	*Z = 0.00;

	GameFactory::Operate<Object, FailPolicy::Return>(id, [X, Y, Z](FactoryObject& object) {
		*X = object->GetNetworkPos(Axis_X);
		*Y = object->GetNetworkPos(Axis_Y);
		*Z = object->GetNetworkPos(Axis_Z);
	});
}

void Script::GetAngle(NetworkID id, double* X, double* Y, double* Z)
{
	*X = 0.00;
	*Y = 0.00;
	*Z = 0.00;

	GameFactory::Operate<Object, FailPolicy::Return>(id, [X, Y, Z](FactoryObject& object) {
		*X = object->GetAngle(Axis_X);
		*Y = object->GetAngle(Axis_Y);
		*Z = object->GetAngle(Axis_Z);
	});
}

unsigned int Script::GetCell(NetworkID id)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [](FactoryObject& object) {
		return object->GetNetworkCell();
	});
}

unsigned int Script::GetLock(NetworkID id)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [](FactoryObject& object) {
		return object->GetLockLevel();
	});
}

unsigned int Script::GetOwner(NetworkID id)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [](FactoryObject& object) {
		return object->GetOwner();
	});
}

const char* Script::GetBaseName(NetworkID id)
{
	static string name;

	return GameFactory::Operate<Object, FailPolicy::Bool>(id, [](FactoryObject& object) {
		name.assign(object->GetName());
	}) ? name.c_str() : "";
}

bool Script::IsNearPoint(NetworkID id, double X, double Y, double Z, double R)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [X, Y, Z, R](FactoryObject& object) {
		return object->IsNearPoint(X, Y, Z, R);
	});
}

NetworkID Script::GetItemContainer(NetworkID id)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
		return item->GetItemContainer();
	});
}

unsigned int Script::GetItemCount(NetworkID id)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
		return item->GetItemCount();
	});
}

double Script::GetItemCondition(NetworkID id)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
		return item->GetItemCondition();
	});
}

bool Script::GetItemEquipped(NetworkID id)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
		return item->GetItemEquipped();
	});
}

bool Script::GetItemSilent(NetworkID id)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
		return item->GetItemSilent();
	});
}

bool Script::GetItemStick(NetworkID id)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
		return item->GetItemStick();
	});
}

unsigned int Script::GetContainerItemCount(NetworkID id, unsigned int baseID)
{
	return GameFactory::Operate<Container, FailPolicy::Return, ObjectPolicy::Expected>(id, [id, baseID](ExpectedContainer& container) {
		if (container || scriptIL.count(id))
		{
			ItemList* IL = container ? &container->IL : scriptIL[id].get();
			return IL->GetItemCount(baseID);
		}

		throw VaultException("Invalid parameters: container or item list doesn't exist").stacktrace();
	});
}

unsigned int Script::GetContainerItemList(NetworkID id, NetworkID** data)
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<Container, FailPolicy::Return, ObjectPolicy::Expected>(id, [id, data](ExpectedContainer& container) {
		if (!container && !scriptIL.count(id))
			return 0u;

		ItemList* IL = container ? &container->IL : scriptIL[id].get();
		_data.assign(IL->GetItemList().begin(), IL->GetItemList().end());
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	});
}

double Script::GetActorValue(NetworkID id, unsigned char index)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [index](FactoryActor& actor) {
		return actor->GetActorValue(index);
	});
}

double Script::GetActorBaseValue(NetworkID id, unsigned char index)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [index](FactoryActor& actor) {
		return actor->GetActorBaseValue(index);
	});
}

unsigned int Script::GetActorIdleAnimation(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorIdleAnimation();
	});
}

unsigned char Script::GetActorMovingAnimation(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorMovingAnimation();
	});
}

unsigned char Script::GetActorWeaponAnimation(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorWeaponAnimation();
	});
}

bool Script::GetActorAlerted(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorAlerted();
	});
}

bool Script::GetActorSneaking(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorSneaking();
	});
}

bool Script::GetActorDead(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorDead();
	});
}

unsigned int Script::GetActorBaseRace(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorRace();
	});
}

bool Script::GetActorBaseSex(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->GetActorFemale();
	});
}

bool Script::IsActorJumping(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [](FactoryActor& actor) {
		return actor->IsActorJumping();
	});
}

unsigned int Script::GetPlayerRespawnTime(NetworkID id)
{
	return GameFactory::Operate<Player, FailPolicy::Return>(id, [](FactoryPlayer& player) {
		return player->GetPlayerRespawnTime();
	});
}

unsigned int Script::GetPlayerSpawnCell(NetworkID id)
{
	return GameFactory::Operate<Player, FailPolicy::Return>(id, [](FactoryPlayer& player) {
		return player->GetPlayerSpawnCell();
	});
}

bool Script::GetPlayerConsoleEnabled(NetworkID id)
{
	return GameFactory::Operate<Player, FailPolicy::Return>(id, [](FactoryPlayer& player) {
		return player->GetPlayerConsoleEnabled();
	});
}

unsigned int Script::GetPlayerWindowCount(NetworkID id)
{
	return GameFactory::Operate<Player, FailPolicy::Return>(id, [](FactoryPlayer& player) {
		return player->GetPlayerWindows().size();
	});
}

unsigned int Script::GetPlayerWindowList(NetworkID id, NetworkID** data)
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<Player, FailPolicy::Return>(id, [data](FactoryPlayer& player) {
		const auto& windows = player->GetPlayerWindows();
		_data.assign(windows.begin(), windows.end());
		unsigned int size = _data.size();

		if (size)
			*data = &_data[0];

		return size;
	});
}

NetworkID Script::GetPlayerChatboxWindow(NetworkID id)
{
	unordered_set<NetworkID> windows;

	return GameFactory::Operate<Player, FailPolicy::Bool>(id, [&windows](FactoryPlayer& player) {
		windows = player->GetPlayerWindows();
	}) ? *find_if(windows.begin(), windows.end(), [](const NetworkID& id) { return IsChatbox(id); }) : 0;
}

NetworkID Script::CreateObject(unsigned int baseID, NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	NetworkID result = 0;
	FactoryObject object;
	FactoryObject _object;

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

	Network::Queue({Network::CreateResponse(
		_object->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	return result;
}

bool Script::DestroyObject(NetworkID id)
{
	return GameFactory::Operate<Object, FailPolicy::Return, ObjectPolicy::Expected>(id, [id](ExpectedObject& object) -> bool {
		if (!object)
			return scriptIL.erase(id);
		else if (vaultcast<Player>(object))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id, true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		if (object->IsPersistent())
			deletedStatic[object->GetNetworkCell()].emplace_back(object->GetReference());

		NetworkID container = GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
			return item->GetItemContainer();
		});

		if (container)
		{
			GameFactory::LeaveReference(object.get());

			GameFactory::Operate<Container>(container, [id](FactoryContainer& container) {
				container->IL.RemoveItem(id);
			});

			return GameFactory::DestroyInstance(id);
		}

		return GameFactory::DestroyInstance(object.get());
	});
}

bool Script::SetPos(NetworkID id, double X, double Y, double Z)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [id, X, Y, Z](FactoryObject& object) {
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

		if (!(static_cast<bool>(object->SetNetworkPos(Axis_X, X)) | static_cast<bool>(object->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(object->SetNetworkPos(Axis_Z, Z))))
			return false;

		object->SetGamePos(Axis_X, X);
		object->SetGamePos(Axis_Y, Y);
		object->SetGamePos(Axis_Z, Z);

		NetworkResponse response;
		unsigned int new_cell_;

		if (new_cell && object->SetNetworkCell((new_cell_ = new_cell->GetBase())))
		{
			object->SetGameCell(new_cell_);

			auto player = vaultcast<Player>(object);

			if (player)
			{
				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_cell->GetWorld(), new_cell->GetX(), new_cell->GetY(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
				);

				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, player->GetPlayerCellContext(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
				);
			}

			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, new_cell_, X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			);
		}
		else
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(id, X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			);

		Network::Queue(move(response));

		return true;
	});
}

bool Script::SetAngle(NetworkID id, double X, double Y, double Z)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [id, X, Y, Z](FactoryObject& object) {
		NetworkResponse response;

		if (object->SetAngle(Axis_X, X))
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(id, Axis_X, X),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			);

		if (object->SetAngle(Axis_Y, Y))
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(id, Axis_Y, Y),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			);

		if (object->SetAngle(Axis_Z, Z))
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(id, Axis_Z, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			);

		if (response.empty())
			return false;

		Network::Queue(move(response));

		return true;
	});
}

bool Script::SetCell(NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	return SetCell_(id, cell, X, Y, Z, false);
}

bool Script::SetCell_(NetworkID id, unsigned int cell, double X, double Y, double Z, bool nosend)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [id, cell, X, Y, Z, nosend](FactoryObject& object) {
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
		auto player = vaultcast<Player>(object);

		if (object->SetNetworkCell(cell))
		{
			object->SetGameCell(cell);

			if (!nosend)
			{
				if (player)
				{
					if (new_interior)
						response.emplace_back(Network::CreateResponse(
							PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(id, DB::Record::Lookup(cell, "CELL")->GetName(), false),
							HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
						);
					else
						response.emplace_back(Network::CreateResponse(
							PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_exterior->GetWorld(), new_exterior->GetX(), new_exterior->GetY(), false),
							HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
						);

					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, player->GetPlayerCellContext(), false),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
					);
				}

				if (update_pos && (static_cast<bool>(object->SetNetworkPos(Axis_X, X)) | static_cast<bool>(object->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(object->SetNetworkPos(Axis_Z, Z))))
				{
					object->SetGamePos(Axis_X, X);
					object->SetGamePos(Axis_Y, Y);
					object->SetGamePos(Axis_Z, Z);

					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell, X, Y, Z),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					);
				}
				else
					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell, 0.0, 0.0, 0.0),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					);
			}

			state = true;
		}

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

		if (!response.empty() && !nosend)
			Network::Queue(move(response));

		return state;
	});
}

bool Script::SetLock(NetworkID id, unsigned int lock)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [id, lock](FactoryObject& object) mutable {
		if (object->GetLockLevel() == Lock_Broken)
			return false;

		if (lock != Lock_Unlocked)
		{
			lock = ceil(static_cast<double>(lock) / 25.0) * 25;

			if (lock > Lock_VeryHard)
				lock = Lock_Impossible;

			if (DB::Terminal::Lookup(object->GetBase()))
			{
				if (lock == Lock_Impossible)
					lock = 5;
				else
					lock /= 25;
			}
		}

		if (!object->SetLockLevel(lock))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_LOCK>(id, lock),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::SetOwner(NetworkID id, unsigned int owner)
{
	return GameFactory::Operate<Object, FailPolicy::Return>(id, [id, owner](FactoryObject& object) {
		if (owner == PLAYER_BASE)
			return false;

		auto npc = DB::NPC::Lookup(owner);

		if (!npc || !object->SetOwner(owner))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_OWNER>(id, owner),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::SetBaseName(NetworkID id, const char* name)
{
	if (!name)
		return false;

	string _name(name);

	if (_name.length() > MAX_PLAYER_NAME)
		return false;

	auto reference = GameFactory::GetObjectTypes(ALL_OBJECTS);
	auto it = find_if(reference.begin(), reference.end(), [id](const FactoryObject& object) { return object->GetNetworkID() == id; });

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
				Network::Queue({Network::CreateResponse(
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
	FactoryObject object;
	FactoryItem item;

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

	Network::Queue({Network::CreateResponse(
		item->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	return result;
}

bool Script::SetItemCount(NetworkID id, unsigned int count)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [id, count](FactoryItem& item) {
		if (!count)
			return false;

		if (!item->SetItemCount(count))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_COUNT>(id, count, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::SetItemCondition(NetworkID id, double condition)
{
	return GameFactory::Operate<Item, FailPolicy::Return>(id, [id, condition](FactoryItem& item) {
		if (condition < 0.00 || condition > 100.0)
			return false;

		auto item_ = DB::Item::Lookup(item->GetBase());

		if (!item_)
			return false;

		if (!item->SetItemCondition(condition))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONDITION>(id, condition, static_cast<unsigned int>(item_->GetHealth() * (condition / 100.0))),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

NetworkID Script::CreateContainer(unsigned int baseID, NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	NetworkID result = 0;
	FactoryObject object;
	FactoryContainer container;

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

	Network::Queue({Network::CreateResponse(
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
	return GameFactory::Operate<Container, FailPolicy::Return, ObjectPolicy::Expected>(id, [id, baseID, count, condition, silent](ExpectedContainer& container) {
		if (!count)
			return false;

		if (!container && !scriptIL.count(id))
			return false;

		ItemList* IL = container ? &container->IL : scriptIL[id].get();
		auto diff = IL->AddItem(baseID, count, condition, silent);

		if (container)
			GameFactory::Operate<Item>(diff.second, [&diff, silent](FactoryItem& item) {
				if (diff.first)
					Network::Queue({Network::CreateResponse(
						item->toPacket(),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});
				else
					Network::Queue({Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_COUNT>(diff.second, item->GetItemCount(), silent),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});
			});

		return true;
	});
}

void Script::AddItemList(NetworkID id, NetworkID source, unsigned int baseID)
{
	GameFactory::Operate<Container, FailPolicy::Return, ObjectPolicy::Expected>(id, [id, source, baseID](ExpectedContainer& container) {
		if (!container && !scriptIL.count(id))
			return;

		if (source)
		{
			if (container)
				GameFactory::LeaveReference(container.get());

			container = GameFactory::GetObject<Container>(source);

			if (!container && !scriptIL.count(source))
				return;

			ItemList* IL = container ? &container->IL : scriptIL[source].get();
			auto items = GameFactory::GetMultiple<Item>(vector<NetworkID>{IL->GetItemList().begin(), IL->GetItemList().end()});

			for (auto& item : items)
			{
				AddItem(id, item->GetBase(), item->GetItemCount(), item->GetItemCondition(), item->GetItemSilent());

				if (item->GetItemEquipped())
					EquipItem(id, item->GetBase(), item->GetItemSilent(), item->GetItemStick());
			}
		}
		else if (container || baseID)
		{
			const auto& items = DB::BaseContainer::Lookup(container ? container->GetBase() : baseID);

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

unsigned int Script::RemoveItem(NetworkID id, unsigned int baseID, unsigned int count, bool silent)
{
	return GameFactory::Operate<Container, FailPolicy::Return, ObjectPolicy::Expected>(id, [id, baseID, count, silent](ExpectedContainer& container) {
		if (!count)
			return 0u;

		if (!container && !scriptIL.count(id))
			return 0u;

		ItemList* IL = container ? &container->IL : scriptIL[id].get();
		auto diff = IL->RemoveItem(baseID, count, silent);

		unsigned int count = get<0>(diff);

		if (!count)
			return 0u;

		if (container)
		{
			const auto& remove = get<1>(diff);
			NetworkID update = get<2>(diff);
			NetworkResponse response;

			for (const auto& id : remove)
				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id, silent),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				);

			if (update)
				GameFactory::Operate<Item>(update, [&response, update, silent](FactoryItem& item) {
					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_COUNT>(update, item->GetItemCount(), silent),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					);
				});

			Network::Queue(move(response));
		}

		return count;
	});
}

void Script::RemoveAllItems(NetworkID id)
{
	GameFactory::Operate<Container, FailPolicy::Return, ObjectPolicy::Expected>(id, [id](ExpectedContainer& container) {
		if (!container && !scriptIL.count(id))
			return;

		ItemList* IL = container ? &container->IL : scriptIL[id].get();
		auto diff = IL->RemoveAllItems();

		if (container)
		{
			NetworkResponse response;

			for (const auto& id : diff)
				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id, true),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				);

			Network::Queue(move(response));
		}
	});
}

NetworkID Script::CreateActor(unsigned int baseID, NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	NetworkID result = 0;
	FactoryObject object;
	FactoryActor actor;

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

	Network::Queue({Network::CreateResponse(
		actor->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
	});

	return result;
}

void Script::SetActorValue(NetworkID id, unsigned char index, double value)
{
	GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, index, value](FactoryActor& actor) {
		if (actor->SetActorValue(index, value))
			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(id, false, index, value),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});
	});
}

void Script::SetActorBaseValue(NetworkID id, unsigned char index, double value)
{
	auto reference = GameFactory::GetObjectTypes<Actor>(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [id](const FactoryActor& actor) { return actor->GetNetworkID() == id; });

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
					Network::Queue({Network::CreateResponse(
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
	return GameFactory::Operate<Actor, FailPolicy::Return, ObjectPolicy::Expected>(id, [id, baseID, silent, stick](ExpectedActor& actor) {
		if (!actor && !scriptIL.count(id))
			return false;

		ItemList* IL = actor ? &actor->IL : scriptIL[id].get();
		auto diff = IL->EquipItem(baseID, silent, stick);

		if (!diff)
			return false;

		if (actor)
			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_EQUIPPED>(diff, true, silent, stick),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

		return true;
	});
}

bool Script::UnequipItem(NetworkID id, unsigned int baseID, bool silent, bool stick)
{
	return GameFactory::Operate<Actor, FailPolicy::Return, ObjectPolicy::Expected>(id, [id, baseID, silent, stick](ExpectedActor& actor) {
		if (!actor && !scriptIL.count(id))
			return false;

		ItemList* IL = actor ? &actor->IL : scriptIL[id].get();
		auto diff = IL->UnequipItem(baseID, silent, stick);

		if (!diff)
			return false;

		if (actor)
			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_EQUIPPED>(diff, false, silent, stick),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

		return true;
	});
}

bool Script::SetActorMovingAnimation(NetworkID id, unsigned char anim)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, anim](FactoryActor& actor) {
		if (vaultcast<Player>(actor))
			return false;

		if (!actor->SetActorMovingAnimation(anim))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), anim, actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), actor->GetActorAlerted(), actor->GetActorSneaking(), false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::SetActorWeaponAnimation(NetworkID id, unsigned char anim)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, anim](FactoryActor& actor) {
		if (vaultcast<Player>(actor))
			return false;

		if (!actor->SetActorWeaponAnimation(anim))
			return false;

		bool punching = actor->IsActorPunching();
		bool power_punching = actor->IsActorPowerPunching();
		bool firing = actor->IsActorFiring();

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), anim, actor->GetActorAlerted(), actor->GetActorSneaking(), !punching && !power_punching && firing),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::SetActorAlerted(NetworkID id, bool alerted)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, alerted](FactoryActor& actor) {
		if (vaultcast<Player>(actor))
			return false;

		if (!actor->SetActorAlerted(alerted))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), alerted, actor->GetActorSneaking(), false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::SetActorSneaking(NetworkID id, bool sneaking)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, sneaking](FactoryActor& actor) {
		if (vaultcast<Player>(actor))
			return false;

		if (!actor->SetActorSneaking(sneaking))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, actor->GetActorIdleAnimation(), actor->GetActorMovingAnimation(), actor->GetActorMovingXY(), actor->GetActorWeaponAnimation(), actor->GetActorAlerted(), sneaking, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::FireWeapon(NetworkID id)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [id](FactoryActor& actor) {
		unsigned int baseID = actor->GetEquippedWeapon();
		auto weapon = DB::Weapon::Lookup(baseID);

		if (!weapon)
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(id, baseID, weapon->IsAutomatic() ? weapon->GetFireRate() : 0.00),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

bool Script::PlayIdle(NetworkID id, unsigned int idle)
{
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, idle](FactoryActor& actor) {
		auto idle_ = DB::Record::Lookup(idle, "IDLE");

		if (!idle_)
			return false;

		if (!actor->SetActorIdleAnimation(idle))
			return false;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_IDLE>(id, idle, idle_->GetName()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		return true;
	});
}

void Script::KillActor(NetworkID id, unsigned short limbs, signed char cause)
{
	GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, limbs, cause](FactoryActor& actor) {
		if (!actor->SetActorDead(true))
			return;

		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, true, limbs, cause),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		});

		auto player = vaultcast<Player>(actor);

		if (player)
			Script::CreateTimerEx(reinterpret_cast<ScriptFunc>(&Script::Timer_Respawn), player->GetPlayerRespawnTime(), "l", id);
	});
}

bool Script::SetActorBaseRace(NetworkID id, unsigned int race)
{
	auto reference = GameFactory::GetObjectTypes<Actor>(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [id](const FactoryActor& actor) { return actor->GetNetworkID() == id; });

	if (it == reference.end())
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
					Network::Queue({Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_RACE>(actor->GetNetworkID(), race, DB::Race::Lookup(RACE_CAUCASIAN)->GetAgeDifference(race), delta_age),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});
				else
					Network::Queue({Network::CreateResponse(
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
	return GameFactory::Operate<Actor, FailPolicy::Return>(id, [id, age](FactoryActor& actor) {
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
	});
}

bool Script::SetActorBaseSex(NetworkID id, bool female)
{
	auto reference = GameFactory::GetObjectTypes<Actor>(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [id](const FactoryActor& actor) { return actor->GetNetworkID() == id; });

	if (it == reference.end())
		return false;

	unsigned int baseID = (*it)->GetBase();

	if (DB::Record::Lookup(baseID, "CREA"))
		return false;

	if (baseID == PLAYER_BASE)
		return false;

	DB::NPC* npc = *DB::NPC::Lookup(baseID);
	bool old_female = npc->IsFemale();

	if (old_female != female)
	{
		npc->SetFemale(female);

		for (const auto& actor : reference)
		{
			if (actor->GetBase() == baseID)
			{
				actor->SetActorFemale(female);

				Network::Queue({Network::CreateResponse(
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
	GameFactory::Operate<Player, FailPolicy::Return>(id, [respawn](FactoryPlayer& player) {
		player->SetPlayerRespawnTime(respawn);
	});
}

void Script::SetPlayerSpawnCell(NetworkID id, unsigned int cell)
{
	GameFactory::Operate<Player, FailPolicy::Return>(id, [id, cell](FactoryPlayer& player) {
		auto cell_ = DB::Exterior::Lookup(cell);

		if (!cell_)
		{
			auto record = DB::Record::Lookup(cell, "CELL");

			if (!record)
				return;

			if (player->SetPlayerSpawnCell(cell))
				Network::Queue({Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(id, record->GetName(), true),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()),

					Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, Player::CellContext{{cell, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}}, true),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
				});
		}
		else if (player->SetPlayerSpawnCell(cell))
			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, cell_->GetWorld(), cell_->GetX(), cell_->GetY(), true),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID()),

				Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, cell_->GetAdjacents(), true),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
			});
	});
}

void Script::SetPlayerConsoleEnabled(NetworkID id, bool enabled)
{
	GameFactory::Operate<Player, FailPolicy::Return>(id, [id, enabled](FactoryPlayer& player) {
		if (player->SetPlayerConsoleEnabled(enabled))
			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONSOLE>(id, enabled),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
			});
	});
}

bool Script::AttachWindow(NetworkID id, NetworkID window)
{
	if (!GameFactory::Operate<Window, FailPolicy::Return>(window, [](FactoryWindow& window) {
		return !window->GetParentWindow();
	})) return false;

	if (!GameFactory::Operate<Player, FailPolicy::Return>(id, [window](FactoryPlayer& player) {
		return player->AttachWindow(window);
	})) return false;

	vector<NetworkID> additions;
	Window::CollectChilds(window, additions);

	NetworkResponse response;

	for (const auto& id_ : additions)
		response.emplace_back(Network::CreateResponse(
			GameFactory::GetObject<Window>(id_)->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		);

	Network::Queue(move(response));

	return true;
}

bool Script::DetachWindow(NetworkID id, NetworkID window)
{
	if (!GameFactory::Operate<Window, FailPolicy::Return>(window, [window](FactoryWindow&) {
		return !IsChatbox(window);
	})) return false;

	if (!GameFactory::Operate<Player, FailPolicy::Return>(id, [window](FactoryPlayer& player) {
		return player->DetachWindow(window);
	})) return false;

	vector<NetworkID> deletions;
	Window::CollectChilds(window, deletions);
	reverse(deletions.begin(), deletions.end()); // reverse so the order of deletion is valid

	NetworkResponse response;

	for (const auto& id_ : deletions)
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_WINDOW_REMOVE>(id_),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		);

	Network::Queue(move(response));

	return true;
}

void Script::ForceWindowMode(NetworkID id, bool enabled)
{
	GameFactory::Operate<Player, FailPolicy::Return>(id, [id, enabled](FactoryPlayer&) {
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WMODE>(enabled),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
		});
	});
}

NetworkID Script::GetParentWindow(NetworkID id)
{
	return GameFactory::Operate<Window, FailPolicy::Return>(id, [id](FactoryWindow& window) {
		return window->GetParentWindow();
	});
}

NetworkID Script::GetWindowRoot(NetworkID id)
{
	return GameFactory::Operate<Window, FailPolicy::Return>(id, [id](FactoryWindow& window) {
		NetworkID parent;

		while ((parent = window->GetParentWindow()))
		{
			GameFactory::LeaveReference(window);
			window = GameFactory::GetObject<Window>(parent).get();
		}

		return window->GetNetworkID();
	});
}

unsigned int Script::GetWindowChildCount(NetworkID id)
{
	return GameFactory::Operate<Window, FailPolicy::Return>(id, [id](FactoryWindow&) {
		const auto& childs = Window::GetChilds();
		return childs.count(id) ? childs.find(id)->second.size() : 0;
	});
}

unsigned int Script::GetWindowChildList(NetworkID id, NetworkID** data)
{
	static vector<NetworkID> _data;

	return GameFactory::Operate<Window, FailPolicy::Return>(id, [id, data](FactoryWindow&) {
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

void Script::GetWindowPos(NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y)
{
	*X = 0.0;
	*Y = 0.0;
	*offset_X = 0.0;
	*offset_Y = 0.0;

	GameFactory::Operate<Window, FailPolicy::Return>(id, [X, Y, offset_X, offset_Y](FactoryWindow& window) {
		const auto& pos = window->GetPos();
		*X = get<0>(pos);
		*Y = get<1>(pos);
		*offset_X = get<2>(pos);
		*offset_Y = get<3>(pos);
	});
}

void Script::GetWindowSize(NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y)
{
	*X = 0.0;
	*Y = 0.0;
	*offset_X = 0.0;
	*offset_Y = 0.0;

	GameFactory::Operate<Window, FailPolicy::Return>(id, [X, Y, offset_X, offset_Y](FactoryWindow& window) {
		const auto& size = window->GetSize();
		*X = get<0>(size);
		*Y = get<1>(size);
		*offset_X = get<2>(size);
		*offset_Y = get<3>(size);
	});
}

bool Script::GetWindowVisible(NetworkID id)
{
	return GameFactory::Operate<Window, FailPolicy::Return>(id, [id](FactoryWindow& window) {
		return window->GetVisible();
	});
}

bool Script::GetWindowLocked(NetworkID id)
{
	return GameFactory::Operate<Window, FailPolicy::Return>(id, [id](FactoryWindow& window) {
		return window->GetLocked();
	});
}

const char* Script::GetWindowText(NetworkID id)
{
	static string text;

	return GameFactory::Operate<Window, FailPolicy::Bool>(id, [id](FactoryWindow& window) {
		text.assign(window->GetText());
	}) ? text.c_str() : "";
}

unsigned int Script::GetEditMaxLength(NetworkID id)
{
	return GameFactory::Operate<Edit, FailPolicy::Return>(id, [id](FactoryEdit& edit) {
		return edit->GetMaxLength();
	});
}

const char* Script::GetEditValidation(NetworkID id)
{
	static string validation;

	return GameFactory::Operate<Edit, FailPolicy::Bool>(id, [id](FactoryEdit& edit) {
		validation.assign(edit->GetValidation());
	}) ? validation.c_str() : "";
}

NetworkID (Script::CreateWindow)(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text)
{
	NetworkID id = GameFactory::CreateInstance(ID_WINDOW, 0x00000000);
	auto window = GameFactory::GetObject<Window>(id);
	SetupWindow(window.get(), posX, posY, offset_posX, offset_posY, sizeX, sizeY, offset_sizeX, offset_sizeY, visible, locked, text);
	return id;
}

bool Script::DestroyWindow(NetworkID id)
{
	NetworkID root = GetWindowRoot(id);

	if (!root || IsChatbox(id))
		return false;

	vector<NetworkID> deletions;
	Window::CollectChilds(id, deletions);
	reverse(deletions.begin(), deletions.end()); // reverse so the order of deletion is valid

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
			{
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());

				if (root == id)
					player->DetachWindow(root);
			}
	}

	NetworkResponse response;

	for (const auto& id : deletions)
	{
		if (!guids.empty())
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_WINDOW_REMOVE>(id),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
			);

		GameFactory::DestroyInstance(id);
	}

	if (!response.empty())
		Network::Queue(move(response));

	return true;
}

bool Script::AddChildWindow(NetworkID id, NetworkID child)
{
	{
		auto windows = GameFactory::GetMultiple<Window>(vector<NetworkID>{id, child});

		if (!windows[0] || !windows[1] || IsChatbox(id) || IsChatbox(child))
			return false;

		if (windows[1]->GetParentWindow())
			return false;

		windows[1]->SetParentWindow(windows[0].get().operator->());
	}

	NetworkID root = GetWindowRoot(id);

	vector<NetworkID> additions;
	Window::CollectChilds(child, additions);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
	{
		NetworkResponse response;

		for (const auto& id : additions)
			response.emplace_back(Network::CreateResponse(
				GameFactory::GetObject<Window>(id)->toPacket(),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
			);

		Network::Queue(move(response));
	}

	return true;
}

bool Script::RemoveChildWindow(NetworkID id, NetworkID child)
{
	{
		auto windows = GameFactory::GetMultiple<Window>(vector<NetworkID>{id, child});

		if (!windows[0] || !windows[1])
			return false;

		if (windows[1]->GetParentWindow() != id)
			return false;

		windows[1]->SetParentWindow(nullptr);
	}

	NetworkID root = GetWindowRoot(id);

	vector<NetworkID> deletions;
	Window::CollectChilds(child, deletions);
	reverse(deletions.begin(), deletions.end()); // reverse so the order of deletion is valid

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
	{
		NetworkResponse response;

		for (const auto& id : deletions)
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_WINDOW_REMOVE>(id),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
			);

		Network::Queue(move(response));
	}

	return true;
}

bool Script::SetWindowPos(NetworkID id, double X, double Y, double offset_X, double offset_Y)
{
	if (!GameFactory::Operate<Window, FailPolicy::Return>(id, [X, Y, offset_X, offset_Y](FactoryWindow& window) {
		return window->SetPos(X, Y, offset_X, offset_Y);
	})) return false;

	NetworkID root = GetWindowRoot(id);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WPOS>(id, tuple<double, double, double, double>{X, Y, offset_X, offset_Y}),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
		});

	return true;
}

bool Script::SetWindowSize(NetworkID id, double X, double Y, double offset_X, double offset_Y)
{
	if (!GameFactory::Operate<Window, FailPolicy::Return>(id, [X, Y, offset_X, offset_Y](FactoryWindow& window) {
		return window->SetSize(X, Y, offset_X, offset_Y);
	})) return false;

	NetworkID root = GetWindowRoot(id);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WSIZE>(id, tuple<double, double, double, double>{X, Y, offset_X, offset_Y}),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
		});

	return true;
}

bool Script::SetWindowVisible(NetworkID id, bool visible)
{
	if (!GameFactory::Operate<Window, FailPolicy::Bool>(id, [visible](FactoryWindow& window) {
		window->SetVisible(visible);
	})) return false;

	NetworkID root = GetWindowRoot(id);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WVISIBLE>(id, visible),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
		});

	return true;
}

bool Script::SetWindowLocked(NetworkID id, bool locked)
{
	if (!GameFactory::Operate<Window, FailPolicy::Bool>(id, [locked](FactoryWindow& window) {
		window->SetLocked(locked);
	})) return false;

	NetworkID root = GetWindowRoot(id);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WLOCKED>(id, locked),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
		});

	return true;
}

bool Script::SetWindowText(NetworkID id, const char* text)
{
	if (!GameFactory::Operate<Window, FailPolicy::Return>(id, [text](FactoryWindow& window) {
		auto edit = vaultcast<Edit>(window);

		if (edit && edit->GetMaxLength() < strlen(text))
			return false;

		window->SetText(text);

		return true;
	})) return false;

	NetworkID root = GetWindowRoot(id);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WTEXT>(id, text),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
		});

	return true;
}

NetworkID Script::CreateButton(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text)
{
	NetworkID id = GameFactory::CreateInstance(ID_BUTTON, 0x00000000);
	auto window = GameFactory::GetObject<Button>(id);
	SetupWindow(window.get(), posX, posY, offset_posX, offset_posY, sizeX, sizeY, offset_sizeX, offset_sizeY, visible, locked, text);
	return id;
}

NetworkID Script::CreateText(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text)
{
	NetworkID id = GameFactory::CreateInstance(ID_TEXT, 0x00000000);
	auto window = GameFactory::GetObject<Text>(id);
	SetupWindow(window.get(), posX, posY, offset_posX, offset_posY, sizeX, sizeY, offset_sizeX, offset_sizeY, visible, locked, text);
	return id;
}

NetworkID Script::CreateEdit(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text)
{
	NetworkID id = GameFactory::CreateInstance(ID_EDIT, 0x00000000);
	auto window = GameFactory::GetObject<Edit>(id);
	SetupWindow(window.get(), posX, posY, offset_posX, offset_posY, sizeX, sizeY, offset_sizeX, offset_sizeY, visible, locked, text);
	return id;
}

bool Script::SetEditMaxLength(NetworkID id, unsigned int length)
{
	bool text_updated = false;

	if (!GameFactory::Operate<Edit, FailPolicy::Bool>(id, [length, &text_updated](FactoryEdit& edit) {
		edit->SetMaxLength(length);

		string text = edit->GetText();

		if (text.length() > length)
		{
			text.resize(length);
			edit->SetText(text);
			text_updated = true;
		}
	})) return false;

	NetworkID root = GetWindowRoot(id);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
	{
		NetworkResponse response;

		if (text_updated)
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_WTEXT>(id, GameFactory::GetObject<Edit>(id)->GetText()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
			);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WMAXLEN>(id, length),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
		);

		Network::Queue(move(response));
	}

	return true;
}

bool Script::SetEditValidation(NetworkID id, const char* validation)
{
	if (!GameFactory::Operate<Edit, FailPolicy::Bool>(id, [validation](FactoryEdit& edit) {
		edit->SetValidation(validation);
	})) return false;

	NetworkID root = GetWindowRoot(id);

	vector<RakNetGUID> guids;

	{
		auto players = GameFactory::GetObjectTypes<Player>(ID_PLAYER);

		for (const auto& player : players)
			if (player->GetPlayerWindows().count(root))
				guids.emplace_back(Client::GetClientFromPlayer(player->GetNetworkID())->GetGUID());
	}

	if (!guids.empty())
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_WVALID>(id, validation),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids)
		});

	return true;
}
