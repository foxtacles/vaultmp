#include "Script.h"

vector<Script*> Script::scripts;
pair<chrono::system_clock::time_point, double> Script::gameTime;
unsigned int Script::gameWeather;

Script::Script(char* path)
{
	FILE* file = fopen(path, "rb");

	if (file == nullptr)
		throw VaultException("Script not found: %s", path);

	fclose(file);

#ifdef __WIN32__
	if (strstr(path, ".dll"))
#else
	if (strstr(path, ".so"))
#endif
	{
		void* handle = nullptr;
#ifdef __WIN32__
		handle = LoadLibrary(path);
#else
		handle = dlopen(path, RTLD_LAZY);
#endif

		if (handle == nullptr)
			throw VaultException("Was not able to load C++ script: %s", path);

		try
		{
			this->handle = handle;
			this->cpp_script = true;

			GetScript("exec", fexec);

			if (!fexec)
				throw VaultException("Could not find exec() callback in: %s", path);

			GetScript("vaultprefix", vaultprefix);
			string vpf(vaultprefix);

			GetScript("OnSpawn", fOnSpawn);
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

			SetScript(string(vpf + "timestamp").c_str(), &Utils::timestamp);
			SetScript(string(vpf + "CreateTimer").c_str(), &Script::CreateTimer);
			SetScript(string(vpf + "CreateTimerEx").c_str(), &Script::CreateTimerEx);
			SetScript(string(vpf + "KillTimer").c_str(), &Script::KillTimer);
			SetScript(string(vpf + "MakePublic").c_str(), &Script::MakePublic);
			SetScript(string(vpf + "CallPublic").c_str(), &Script::CallPublic);

			SetScript(string(vpf + "SetServerName").c_str(), &Dedicated::SetServerName);
			SetScript(string(vpf + "SetServerMap").c_str(), &Dedicated::SetServerMap);
			SetScript(string(vpf + "SetServerRule").c_str(), &Dedicated::SetServerRule);
			SetScript(string(vpf + "GetGameCode").c_str(), &Dedicated::GetGameCode);
			SetScript(string(vpf + "GetMaximumPlayers").c_str(), &Dedicated::GetMaximumPlayers);
			SetScript(string(vpf + "GetCurrentPlayers").c_str(), &Dedicated::GetCurrentPlayers);

			SetScript(string(vpf + "ValueToString").c_str(), &Script::ValueToString);
			SetScript(string(vpf + "AxisToString").c_str(), &Script::AxisToString);
			SetScript(string(vpf + "AnimToString").c_str(), &Script::AnimToString);
			SetScript(string(vpf + "BaseToString").c_str(), &Script::BaseToString);

			SetScript(string(vpf + "UIMessage").c_str(), &Script::UIMessage);
			SetScript(string(vpf + "ChatMessage").c_str(), &Script::ChatMessage);
			SetScript(string(vpf + "SetRespawn").c_str(), &Script::SetRespawn);
			SetScript(string(vpf + "SetSpawnCell").c_str(), &Script::SetSpawnCell);
			SetScript(string(vpf + "SetGameWeather").c_str(), &Script::SetGameWeather);
			SetScript(string(vpf + "SetGameTime").c_str(), &Script::SetGameTime);
			SetScript(string(vpf + "SetGameYear").c_str(), &Script::SetGameYear);
			SetScript(string(vpf + "SetGameMonth").c_str(), &Script::SetGameMonth);
			SetScript(string(vpf + "SetGameDay").c_str(), &Script::SetGameDay);
			SetScript(string(vpf + "SetGameHour").c_str(), &Script::SetGameHour);
			SetScript(string(vpf + "SetTimeScale").c_str(), &Script::SetTimeScale);
			SetScript(string(vpf + "IsValid").c_str(), &Script::IsValid);
			SetScript(string(vpf + "IsObject").c_str(), &Script::IsObject);
			SetScript(string(vpf + "IsItem").c_str(), &Script::IsItem);
			SetScript(string(vpf + "IsContainer").c_str(), &Script::IsContainer);
			SetScript(string(vpf + "IsActor").c_str(), &Script::IsActor);
			SetScript(string(vpf + "IsPlayer").c_str(), &Script::IsPlayer);
			SetScript(string(vpf + "IsCell").c_str(), &Record::IsValidCell);
			SetScript(string(vpf + "IsInterior").c_str(), &Script::IsInterior);
			SetScript(string(vpf + "GetType").c_str(), (unsigned char(*)(NetworkID)) &GameFactory::GetType);
			SetScript(string(vpf + "GetConnection").c_str(), &Script::GetConnection);
			SetScript(string(vpf + "GetCount").c_str(), &GameFactory::GetObjectCount);
			SetScript(string(vpf + "GetList").c_str(), &Script::GetList);
			SetScript(string(vpf + "GetGameWeather").c_str(), &Script::GetGameWeather);
			SetScript(string(vpf + "GetGameTime").c_str(), &Script::GetGameTime);
			SetScript(string(vpf + "GetGameYear").c_str(), &Script::GetGameYear);
			SetScript(string(vpf + "GetGameMonth").c_str(), &Script::GetGameMonth);
			SetScript(string(vpf + "GetGameDay").c_str(), &Script::GetGameDay);
			SetScript(string(vpf + "GetGameHour").c_str(), &Script::GetGameHour);
			SetScript(string(vpf + "GetTimeScale").c_str(), &Script::GetTimeScale);

			SetScript(string(vpf + "GetReference").c_str(), &Script::GetReference);
			SetScript(string(vpf + "GetBase").c_str(), &Script::GetBase);
			SetScript(string(vpf + "GetName").c_str(), &Script::GetName);
			SetScript(string(vpf + "GetPos").c_str(), &Script::GetPos);
			SetScript(string(vpf + "GetAngle").c_str(), &Script::GetAngle);
			SetScript(string(vpf + "GetCell").c_str(), &Script::GetCell);
			SetScript(string(vpf + "IsNearPoint").c_str(), &Script::IsNearPoint);
			SetScript(string(vpf + "GetItemContainer").c_str(), &Script::GetItemContainer);
			SetScript(string(vpf + "GetItemCount").c_str(), &Script::GetItemCount);
			SetScript(string(vpf + "GetItemCondition").c_str(), &Script::GetItemCondition);
			SetScript(string(vpf + "GetItemEquipped").c_str(), &Script::GetItemEquipped);
			SetScript(string(vpf + "GetItemSilent").c_str(), &Script::GetItemSilent);
			SetScript(string(vpf + "GetItemStick").c_str(), &Script::GetItemStick);
			SetScript(string(vpf + "GetContainerItemCount").c_str(), &Script::GetContainerItemCount);
			SetScript(string(vpf + "GetActorValue").c_str(), &Script::GetActorValue);
			SetScript(string(vpf + "GetActorBaseValue").c_str(), &Script::GetActorBaseValue);
			SetScript(string(vpf + "GetActorIdleAnimation").c_str(), &Script::GetActorIdleAnimation);
			SetScript(string(vpf + "GetActorMovingAnimation").c_str(), &Script::GetActorMovingAnimation);
			SetScript(string(vpf + "GetActorWeaponAnimation").c_str(), &Script::GetActorWeaponAnimation);
			SetScript(string(vpf + "GetActorAlerted").c_str(), &Script::GetActorAlerted);
			SetScript(string(vpf + "GetActorSneaking").c_str(), &Script::GetActorSneaking);
			SetScript(string(vpf + "GetActorDead").c_str(), &Script::GetActorDead);
			SetScript(string(vpf + "GetActorBaseRace").c_str(), &Script::GetActorBaseRace);
			SetScript(string(vpf + "GetActorBaseSex").c_str(), &Script::GetActorBaseSex);
			SetScript(string(vpf + "IsActorJumping").c_str(), &Script::IsActorJumping);
			SetScript(string(vpf + "GetPlayerRespawn").c_str(), &Script::GetPlayerRespawn);
			SetScript(string(vpf + "GetPlayerSpawnCell").c_str(), &Script::GetPlayerSpawnCell);

			SetScript(string(vpf + "SetPos").c_str(), &Script::SetPos);
			SetScript(string(vpf + "SetCell").c_str(), &Script::SetCell);
			SetScript(string(vpf + "AddItem").c_str(), &Script::AddItem);
			SetScript(string(vpf + "RemoveItem").c_str(), &Script::RemoveItem);
			SetScript(string(vpf + "RemoveAllItems").c_str(), &Script::RemoveAllItems);
			SetScript(string(vpf + "SetActorValue").c_str(), &Script::SetActorValue);
			SetScript(string(vpf + "SetActorBaseValue").c_str(), &Script::SetActorBaseValue);
			SetScript(string(vpf + "EquipItem").c_str(), &Script::EquipItem);
			SetScript(string(vpf + "UnequipItem").c_str(), &Script::UnequipItem);
			SetScript(string(vpf + "PlayIdle").c_str(), &Script::PlayIdle);
			SetScript(string(vpf + "KillActor").c_str(), &Script::KillActor);
			SetScript(string(vpf + "SetActorBaseRace").c_str(), &Script::SetActorBaseRace);
			SetScript(string(vpf + "AgeActorBaseRace").c_str(), &Script::AgeActorBaseRace);
			SetScript(string(vpf + "SetActorBaseSex").c_str(), &Script::SetActorBaseSex);
			SetScript(string(vpf + "SetPlayerRespawn").c_str(), &Script::SetPlayerRespawn);
			SetScript(string(vpf + "SetPlayerSpawnCell").c_str(), &Script::SetPlayerSpawnCell);
		}
		catch (...)
		{
#ifdef __WIN32__
			FreeLibrary((HINSTANCE) handle);
#else
			dlclose(handle);
#endif
			throw;
		}
	}
	else if (strstr(path, ".amx"))
	{
		AMX* vaultscript;

		try
		{
			vaultscript = new AMX();

			this->handle = reinterpret_cast<void*>(vaultscript);
			this->cpp_script = false;

			cell ret = 0;
			int err = 0;

			err = PAWN::LoadProgram(vaultscript, path, nullptr);

			if (err != AMX_ERR_NONE)
				throw VaultException("PAWN script %s error (%d): \"%s\"", path, err, aux_StrError(err));

			PAWN::CoreInit(vaultscript);
			PAWN::ConsoleInit(vaultscript);
			PAWN::FloatInit(vaultscript);
			PAWN::StringInit(vaultscript);
			PAWN::FileInit(vaultscript);
			PAWN::TimeInit(vaultscript);

			err = PAWN::RegisterVaultmpFunctions(vaultscript);

			if (err != AMX_ERR_NONE)
				throw VaultException("PAWN script %s error (%d): \"%s\"", path, err, aux_StrError(err));
		}
		catch (...)
		{
			PAWN::FreeProgram(vaultscript);
			delete vaultscript;
			throw;
		}
	}
	else
		throw VaultException("Script type not recognized: %s", path);
}

Script::~Script()
{
	if (this->cpp_script)
	{
#ifdef __WIN32__
		FreeLibrary((HINSTANCE) this->handle);
#else
		dlclose(this->handle);
#endif
	}
	else
	{
		AMX* vaultscript = reinterpret_cast<AMX*>(this->handle);
		PAWN::FreeProgram(vaultscript);
		delete vaultscript;
	}
}

void Script::LoadScripts(char* scripts, char* base)
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

void Script::Run()
{
	for (Script* script : scripts)
	{
		if (script->cpp_script)
			script->fexec();
		else
		{
			cell ret;
			int err = PAWN::Exec(reinterpret_cast<AMX*>(script->handle), &ret, AMX_EXEC_MAIN);

			if (err != AMX_ERR_NONE)
				throw VaultException("PAWN script error (%d): \"%s\"", err, aux_StrError(err));
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
}

void Script::GetArguments(vector<boost::any>& params, va_list args, string def)
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
					throw VaultException("C++ call: Unknown argument identifier %02X", c);
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
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		KillTimer(); // Player has already left the server
		return 0;
	}

	Player* player = vaultcast<Player>(reference);
	RakNetGUID guid = Client::GetClientFromPlayer(id)->GetGUID();

	RemoveAllItems(id);

	for (const auto& item : Player::default_items)
	{
		AddItem(id, item.first, get<0>(item.second), get<1>(item.second), get<3>(item.second));

		if (get<2>(item.second))
			EquipItem(id, item.first, get<3>(item.second), get<4>(item.second));
	}

	const auto& values = (API::GetGameCode() == FALLOUT3) ? Player::f3_default_values : Player::fnv_default_values;

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

void Script::OnSpawn(const FactoryObject& reference)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnSpawn)
				script->fOnSpawn(id);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnSpawn"))
			PAWN::Call((AMX*)script->handle, "OnSpawn", "l", 0, id);
	}
}

void Script::OnCellChange(const FactoryObject& reference, unsigned int cell)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnCellChange)
				script->fOnCellChange(id, cell);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnCellChange"))
			PAWN::Call((AMX*)script->handle, "OnCellChange", "il", 0, cell, id);
	}
}

void Script::OnContainerItemChange(const FactoryObject& reference, unsigned int baseID, signed int count, double condition)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnContainerItemChange)
				script->fOnContainerItemChange(id, baseID, count, condition);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnContainerItemChange"))
			PAWN::Call((AMX*)script->handle, "OnContainerItemChange", "fiil", 0, condition, count, baseID, id);
	}
}

void Script::OnActorValueChange(const FactoryObject& reference, unsigned char index, bool base, double value)
{
	NetworkID id = reference->GetNetworkID();

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
				if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorBaseValueChange"))
					PAWN::Call((AMX*)script->handle, "OnActorBaseValueChange", "fil", 0, value, (unsigned int) index, id);
			}
			else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorValueChange"))
				PAWN::Call((AMX*)script->handle, "OnActorValueChange", "fil", 0, value, (unsigned int) index, id);
		}
	}
}

void Script::OnActorAlert(const FactoryObject& reference, bool alerted)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorAlert)
				script->fOnActorAlert(id, alerted);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorAlert"))
			PAWN::Call((AMX*)script->handle, "OnActorAlert", "il", 0, (unsigned int) alerted, id);
	}
}

void Script::OnActorSneak(const FactoryObject& reference, bool sneaking)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorSneak)
				script->fOnActorSneak(id, sneaking);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorSneak"))
			PAWN::Call((AMX*)script->handle, "OnActorSneak", "il", 0, (unsigned int) sneaking, id);
	}
}

void Script::OnActorDeath(const FactoryObject& reference, unsigned short limbs, signed char cause)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorDeath)
				script->fOnActorDeath(id, limbs, cause);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorDeath"))
			PAWN::Call((AMX*)script->handle, "OnActorDeath", "iil", 0, cause, limbs, id);
	}
}

void Script::OnActorEquipItem(const FactoryObject& reference, unsigned int baseID, double condition)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorEquipItem)
				script->fOnActorEquipItem(id, baseID, condition);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorEquipItem"))
			PAWN::Call((AMX*)script->handle, "OnActorEquipItem", "fil", 0, condition, baseID, id);
	}
}

void Script::OnActorUnequipItem(const FactoryObject& reference, unsigned int baseID, double condition)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorUnequipItem)
				script->fOnActorUnequipItem(id, baseID, condition);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorUnequipItem"))
			PAWN::Call((AMX*)script->handle, "OnActorUnequipItem", "fil", 0, condition, baseID, id);
	}
}

void Script::OnActorDropItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, double condition)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorDropItem)
				script->fOnActorDropItem(id, baseID, count, condition);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorDropItem"))
			PAWN::Call((AMX*)script->handle, "OnActorDropItem", "fiil", 0, condition, count, baseID, id);
	}
}

void Script::OnActorPickupItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, double condition)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorPickupItem)
				script->fOnActorPickupItem(id, baseID, count, condition);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorPickupItem"))
			PAWN::Call((AMX*)script->handle, "OnActorPickupItem", "fiil", 0, condition, count, baseID, id);
	}
}

void Script::OnActorPunch(const FactoryObject& reference, bool power)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorPunch)
				script->fOnActorPunch(id, power);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorPunch"))
			PAWN::Call((AMX*)script->handle, "OnActorPunch", "il", 0, power, id);
	}
}

void Script::OnActorFireWeapon(const FactoryObject& reference, unsigned int weapon)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorFireWeapon)
				script->fOnActorFireWeapon(id, weapon);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorFireWeapon"))
			PAWN::Call((AMX*)script->handle, "OnActorFireWeapon", "il", 0, weapon, id);
	}
}

void Script::OnPlayerDisconnect(const FactoryObject& reference, Reason reason)
{
	NetworkID id = reference->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnPlayerDisconnect)
				script->fOnPlayerDisconnect(id, reason);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnPlayerDisconnect"))
			PAWN::Call((AMX*)script->handle, "OnPlayerDisconnect", "il", 0, (unsigned int) reason, id);
	}
}

unsigned int Script::OnPlayerRequestGame(const FactoryObject& reference)
{
	NetworkID id = reference->GetNetworkID();
	unsigned int result = 0;

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnPlayerRequestGame)
				result = script->fOnPlayerRequestGame(id);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnPlayerRequestGame"))
			result = (unsigned int) PAWN::Call((AMX*)script->handle, "OnPlayerRequestGame", "l", 0, id);
	}

	return result;
}

bool Script::OnPlayerChat(const FactoryObject& reference, string& message)
{
	NetworkID id = reference->GetNetworkID();
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
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnPlayerChat"))
			result = static_cast<bool>(PAWN::Call((AMX*)script->handle, "OnPlayerChat", "sl", 1, _message, id));
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
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnClientAuthenticate"))
			result = static_cast<bool>(PAWN::Call((AMX*)script->handle, "OnClientAuthenticate", "ss", 0, pwd.c_str(), name.c_str()));
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
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnGameYearChange"))
			PAWN::Call((AMX*)script->handle, "OnGameYearChange", "i", 0, year);
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
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnGameMonthChange"))
			PAWN::Call((AMX*)script->handle, "OnGameMonthChange", "i", 0, month);
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
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnGameDayChange"))
			PAWN::Call((AMX*)script->handle, "OnGameDayChange", "i", 0, day);
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
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnGameHourChange"))
			PAWN::Call((AMX*)script->handle, "OnGameHourChange", "i", 0, hour);
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

	try
	{
		const Record& record = Record::Lookup(baseID);
		base.assign(record.GetName());
	}
	catch (...) {}

	return base.c_str();
}

bool Script::UIMessage(NetworkID id, const char* message)
{
	string _message(message);

	if (_message.length() > MAX_MESSAGE_LENGTH)
		_message.resize(MAX_MESSAGE_LENGTH);

	if (id)
	{
		try
		{
			if (!vaultcast<Player>(GameFactory::GetObject(id)))
				return false;
		}
		catch (...)
		{
			return false;
		}
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_MESSAGE>(_message),
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
		try
		{
			if (!vaultcast<Player>(GameFactory::GetObject(id)))
				return false;
		}
		catch (...)
		{
			return false;
		}
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_CHAT>(_message),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(nullptr))
	});

	return true;
}

void Script::SetRespawn(unsigned int respawn)
{
	Player::SetRespawn(respawn);
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
	if (Record::IsValidWeather(weather))
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
	return GameFactory::GetType(id);
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
	try
	{
		const Exterior& _cell = Exterior::Lookup(cell);
		return false;
	}
	catch (...)
	{
		try
		{
			const Record& record = Record::Lookup(cell, "CELL");
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
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

unsigned int Script::GetReference(NetworkID id)
{
	unsigned int value = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Object* object = vaultcast<Object>(reference);

	if (object)
		value = object->GetReference();

	return value;
}

unsigned int Script::GetBase(NetworkID id)
{
	unsigned int value = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Object* object = vaultcast<Object>(reference);

	if (object)
		value = object->GetBase();

	return value;
}

const char* Script::GetName(NetworkID id)
{
	static string name;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return "";
	}

	Object* object = vaultcast<Object>(reference);

	if (object)
		name.assign(object->GetName());

	return name.c_str();
}

void Script::GetPos(NetworkID id, double* X, double* Y, double* Z)
{
	*X = 0.00;
	*Y = 0.00;
	*Z = 0.00;

	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return;
	}

	Object* object = vaultcast<Object>(reference);

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

	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return;
	}

	Object* object = vaultcast<Object>(reference);

	if (object)
	{
		*X = object->GetAngle(Axis_X);
		*Y = object->GetAngle(Axis_Y);
		*Z = object->GetAngle(Axis_Z);
	}
}

unsigned int Script::GetCell(NetworkID id)
{
	unsigned int value = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Object* object = vaultcast<Object>(reference);

	if (object)
		value = object->GetNetworkCell();

	return value;
}

bool Script::IsNearPoint(NetworkID id, double X, double Y, double Z, double R)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return false;
	}

	Object* object = vaultcast<Object>(reference);

	if (object)
		return object->IsNearPoint(X, Y, Z, R);

	return false;
}

NetworkID Script::GetItemContainer(NetworkID id)
{
	NetworkID container = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return container;
	}

	Item* item = vaultcast<Item>(reference);

	if (item)
		container = item->GetItemContainer();

	return container;
}

unsigned int Script::GetItemCount(NetworkID id)
{
	unsigned int value = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Item* item = vaultcast<Item>(reference);

	if (item)
		value = item->GetItemCount();

	return value;
}

double Script::GetItemCondition(NetworkID id)
{
	double value = 0.00;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Item* item = vaultcast<Item>(reference);

	if (item)
		value = item->GetItemCondition();

	return value;
}

bool Script::GetItemEquipped(NetworkID id)
{
	bool value = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Item* item = vaultcast<Item>(reference);

	if (item)
		value = item->GetItemEquipped();

	return value;
}

bool Script::GetItemSilent(NetworkID id)
{
	bool value = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Item* item = vaultcast<Item>(reference);

	if (item)
		value = item->GetItemSilent();

	return value;
}

bool Script::GetItemStick(NetworkID id)
{
	bool value = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Item* item = vaultcast<Item>(reference);

	if (item)
		value = item->GetItemStick();

	return value;
}

unsigned int Script::GetContainerItemCount(NetworkID id, unsigned int baseID)
{
	unsigned int value = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Container* container = vaultcast<Container>(reference);

	if (container)
		value = container->GetItemCount(baseID);

	return value;
}

double Script::GetActorValue(NetworkID id, unsigned char index)
{
	double value = 0.00;

	try
	{
		FactoryObject reference = GameFactory::GetObject(id);
		Actor* actor = vaultcast<Actor>(reference);

		if (actor)
			value = actor->GetActorValue(index);
	}
	catch (...) {}

	return value;
}

double Script::GetActorBaseValue(NetworkID id, unsigned char index)
{
	double value = 0.00;

	try
	{
		FactoryObject reference = GameFactory::GetObject(id);
		Actor* actor = vaultcast<Actor>(reference);

		if (actor)
			value = actor->GetActorBaseValue(index);
	}
	catch (...) {}

	return value;
}

unsigned int Script::GetActorIdleAnimation(NetworkID id)
{
	unsigned int idle = 0x00000000;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return idle;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		idle = actor->GetActorIdleAnimation();

	return idle;
}

unsigned char Script::GetActorMovingAnimation(NetworkID id)
{
	unsigned char index = 0x00;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return index;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		index = actor->GetActorMovingAnimation();

	return index;
}

unsigned char Script::GetActorWeaponAnimation(NetworkID id)
{
	unsigned char index = 0x00;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return index;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		index = actor->GetActorWeaponAnimation();

	return index;
}

bool Script::GetActorAlerted(NetworkID id)
{
	bool state = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return state;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		state = actor->GetActorAlerted();

	return state;
}

bool Script::GetActorSneaking(NetworkID id)
{
	bool state = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return state;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		state = actor->GetActorSneaking();

	return state;
}

bool Script::GetActorDead(NetworkID id)
{
	bool state = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return state;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		state = actor->GetActorDead();

	return state;
}

unsigned int Script::GetActorBaseRace(NetworkID id)
{
	unsigned int race = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return race;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		race = actor->GetActorRace();

	return race;
}

bool Script::GetActorBaseSex(NetworkID id)
{
	bool female = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return female;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		female = actor->GetActorFemale();

	return female;
}

bool Script::IsActorJumping(NetworkID id)
{
	bool state = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return state;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
		state = actor->IsActorJumping();

	return state;
}

unsigned int Script::GetPlayerRespawn(NetworkID id)
{
	unsigned int value = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Player* player = vaultcast<Player>(reference);

	if (player)
		value = player->GetPlayerRespawn();

	return value;
}

unsigned int Script::GetPlayerSpawnCell(NetworkID id)
{
	unsigned int value = 0;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return value;
	}

	Player* player = vaultcast<Player>(reference);

	if (player)
		value = player->GetPlayerSpawnCell();

	return value;
}

bool Script::SetPos(NetworkID id, double X, double Y, double Z)
{
	bool state = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return state;
	}

	Object* object = vaultcast<Object>(reference);
	unsigned int cell = object->GetNetworkCell();
	const Exterior* new_cell = nullptr;

	try
	{
		unsigned int world = Exterior::Lookup(cell).GetWorld();

		try
		{
			new_cell = &Exterior::Lookup(world, X, Y);
		}
		catch (...)
		{
			return state;
		}
	}
	catch (...) {} // interior, can't check pos (yet? which are the bounds of interiors?)

	NetworkResponse response;
	unsigned int _new_cell = 0x00000000;

	if (object->SetNetworkPos(Axis_X, X) || object->SetNetworkPos(Axis_Y, Y) || object->SetNetworkPos(Axis_Z, Z))
	{
		object->SetGamePos(Axis_X, X);
		object->SetGamePos(Axis_Y, Y);
		object->SetGamePos(Axis_Z, Z);

		Player* player = vaultcast<Player>(reference);

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

				if (player)
				{
					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_cell->GetWorld(), new_cell->GetX(), new_cell->GetY(), false),
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

bool Script::SetCell(NetworkID id, unsigned int cell, double X, double Y, double Z)
{
	bool state = false;
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return state;
	}

	Object* object = vaultcast<Object>(reference);
	bool update_pos = X != 0.00 && Y != 0.00 && Z != 0.00;
	const Record* new_interior = nullptr;
	const Exterior* new_exterior = nullptr;

	try
	{
		if (update_pos)
		{
			unsigned int world = Exterior::Lookup(cell).GetWorld();

			try
			{
				new_exterior = &Exterior::Lookup(world, X, Y);

				if (new_exterior->GetBase() != cell)
					return state;
			}
			catch (...)
			{
				return state;
			}
		}
		else
			new_exterior = &Exterior::Lookup(cell);
	}
	catch (...)
	{
		try
		{
			new_interior = &Record::Lookup(cell, "CELL");
		}
		catch (...)
		{
			return state;
		}
	}

	NetworkResponse response;

	Player* player = vaultcast<Player>(reference);

	if (object->SetNetworkCell(cell))
	{
		object->SetGameCell(cell);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		);

		if (player)
		{
			if (new_interior)
			{
				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(id, new_interior->GetName(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
				);
			}
			else
			{
				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, new_exterior->GetWorld(), new_exterior->GetX(), new_exterior->GetY(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
				);
			}
		}

		state = true;
	}
	else
		cell = 0x00000000;

	if (update_pos && (object->SetNetworkPos(Axis_X, X) || object->SetNetworkPos(Axis_Y, Y) || object->SetNetworkPos(Axis_Z, Z)))
	{
		object->SetGamePos(Axis_X, X);
		object->SetGamePos(Axis_Y, Y);
		object->SetGamePos(Axis_Z, Z);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_POS>(id, X, Y, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		);

		state = true;
	}

	if (state)
		Network::Queue(move(response));

	return state;
}

bool Script::AddItem(NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent)
{
	if (count)
	{
		try
		{
			FactoryObject reference = GameFactory::GetObject(id);
			Container* container = vaultcast<Container>(reference);

			if (container)
			{
				ContainerDiff diff = container->AddItem(baseID, count, condition, silent);

				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, Container::ToNetDiff(diff), ContainerDiffNet()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});

				container->ApplyDiff(diff);

				return true;
			}
		}
		catch (...) {}
	}

	return false;
}

unsigned int Script::RemoveItem(NetworkID id, unsigned int baseID, unsigned int count, bool silent)
{
	unsigned int removed = 0;

	if (count)
	{
		try
		{
			FactoryObject reference = GameFactory::GetObject(id);
			Container* container = vaultcast<Container>(reference);

			if (container)
			{
				ContainerDiff diff = container->RemoveItem(baseID, count, silent);

				if (!diff.first.empty() || !diff.second.empty())
				{
					Network::Queue(NetworkResponse{Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, Container::ToNetDiff(diff), ContainerDiffNet()),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});

					GameDiff gamediff = container->ApplyDiff(diff);

					removed = abs(gamediff.front().second.count);
				}
			}
		}
		catch (...) {}
	}

	return removed;
}

void Script::RemoveAllItems(NetworkID id)
{
	try
	{
		FactoryObject reference = GameFactory::GetObject(id);
		Container* container = vaultcast<Container>(reference);

		if (container)
		{
			ContainerDiff diff = container->RemoveAllItems();

			if (!diff.first.empty() || !diff.second.empty())
			{
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, Container::ToNetDiff(diff), ContainerDiffNet()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});

				container->ApplyDiff(diff);
			}
		}
	}
	catch (...) {}
}

void Script::SetActorValue(NetworkID id, unsigned char index, double value)
{
	try
	{
		FactoryObject reference = GameFactory::GetObject(id);
		Actor* actor = vaultcast<Actor>(reference);

		if (actor)
		{
			if (actor->SetActorValue(index, value))
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(actor->GetNetworkID(), false, index, value),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});
		}
	}
	catch (...) {}
}

void Script::SetActorBaseValue(NetworkID id, unsigned char index, double value)
{
	try
	{
		vector<FactoryObject> reference = GameFactory::GetObjectTypes(ALL_ACTORS);
		auto it = find_if(reference.begin(), reference.end(), [&](const FactoryObject& reference) { return vaultcast<Actor>(reference)->GetNetworkID() == id; });

		if (it == reference.end())
			return;

		unsigned int baseID = vaultcast<Actor>(*it)->GetBase();

		if (baseID == PLAYER_BASE)
			return;

		for (const FactoryObject& _reference : reference)
		{
			Actor* actor = vaultcast<Actor>(_reference);

			if (actor->GetBase() == baseID)
			{
				if (actor->SetActorBaseValue(index, value))
					Network::Queue(NetworkResponse{Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(actor->GetNetworkID(), true, index, value),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
					});
			}
		}
	}
	catch (...) {}
}

bool Script::EquipItem(NetworkID id, unsigned int baseID, bool silent, bool stick)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return false;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
	{
		ContainerDiff diff = actor->EquipItem(baseID, silent, stick);

		if (!diff.first.empty() || !diff.second.empty())
		{
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, Container::ToNetDiff(diff), ContainerDiffNet()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			actor->ApplyDiff(diff);

			return true;
		}
	}

	return false;
}

bool Script::UnequipItem(NetworkID id, unsigned int baseID, bool silent, bool stick)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return false;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
	{
		ContainerDiff diff = actor->UnequipItem(baseID, silent, stick);

		if (!diff.first.empty() || !diff.second.empty())
		{
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, Container::ToNetDiff(diff), ContainerDiffNet()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			actor->ApplyDiff(diff);

			return true;
		}
	}

	return false;
}

bool Script::PlayIdle(NetworkID id, unsigned int idle)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return false;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
	{
		const Record* record = nullptr;

		try
		{
			record = &Record::Lookup(idle, "IDLE");
		}
		catch (...)
		{
			return false;
		}

		if (actor->SetActorIdleAnimation(idle))
		{
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_IDLE>(actor->GetNetworkID(), idle, record->GetName()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			return true;
		}
	}

	return false;
}

void Script::KillActor(NetworkID id, unsigned short limbs, signed char cause)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
	{
		if (actor->SetActorDead(true))
		{
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(actor->GetNetworkID(), true, limbs, cause),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			Player* player = vaultcast<Player>(reference);

			if (player)
				Script::CreateTimerEx(reinterpret_cast<ScriptFunc>(&Script::Timer_Respawn), player->GetPlayerRespawn(), "l", player->GetNetworkID());
		}
	}
}

bool Script::SetActorBaseRace(NetworkID id, unsigned int race)
{
	vector<FactoryObject> reference = GameFactory::GetObjectTypes(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [&](const FactoryObject& reference) { return vaultcast<Actor>(reference)->GetNetworkID() == id; });

	if (it == reference.end())
		return false;

	unsigned int baseID = vaultcast<Actor>(*it)->GetBase();

	if (baseID == PLAYER_BASE)
		return false;

	try
	{
		const Race& _race = Race::Lookup(race);
		const NPC& npc = NPC::Lookup(baseID);
		unsigned int old_race = npc.GetRace();

		if (old_race != race)
		{
			npc.SetRace(race);
			signed int delta_age = Race::Lookup(old_race).GetAgeDifference(race);
			signed int new_age = Race::Lookup(npc.GetOriginalRace()).GetAgeDifference(race);

			for (const FactoryObject& _reference : reference)
			{
				Actor* actor = vaultcast<Actor>(_reference);

				if (actor->GetBase() == baseID)
				{
					actor->SetActorRace(race);
					actor->SetActorAge(new_age);

					if (vaultcast<Player>(_reference))
						Network::Queue(NetworkResponse{Network::CreateResponse(
							PacketFactory::Create<pTypes::ID_UPDATE_RACE>(actor->GetNetworkID(), race, Race::Lookup(RACE_CAUCASIAN).GetAgeDifference(race), delta_age),
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
	catch (...) {}

	return false;
}

bool Script::AgeActorBaseRace(NetworkID id, signed int age)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return false;
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
	{
		const Race* race = &Race::Lookup(actor->GetActorRace());
		unsigned int new_race;

		if (age < 0)
		{
			age = abs(age);

			for (unsigned int i = 0; i < age; ++i)
			{
				new_race = race->GetYounger();

				if (!new_race)
					return false;

				race = &Race::Lookup(new_race);
			}
		}
		else if (age > 0)
		{
			for (unsigned int i = 0; i < age; ++i)
			{
				new_race = race->GetOlder();

				if (!new_race)
					return false;

				race = &Race::Lookup(new_race);
			}
		}
		else
			return true;

		GameFactory::LeaveReference(reference);

		return SetActorBaseRace(id, new_race);
	}

	return false;
}

bool Script::SetActorBaseSex(NetworkID id, bool female)
{
	vector<FactoryObject> reference = GameFactory::GetObjectTypes(ALL_ACTORS);
	auto it = find_if(reference.begin(), reference.end(), [&](const FactoryObject& reference) { return vaultcast<Actor>(reference)->GetNetworkID() == id; });

	if (it == reference.end())
		return false;

	unsigned int baseID = vaultcast<Actor>(*it)->GetBase();

	if (baseID == PLAYER_BASE)
		return false;

	try
	{
		const NPC& npc = NPC::Lookup(baseID);
		bool old_female = npc.IsFemale();

		if (old_female != female)
		{
			npc.SetFemale(female);

			for (const FactoryObject& _reference : reference)
			{
				Actor* actor = vaultcast<Actor>(_reference);

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
	catch (...) {}

	return false;
}

void Script::SetPlayerRespawn(NetworkID id, unsigned int respawn)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return;
	}

	Player* player = vaultcast<Player>(reference);

	if (player)
		player->SetPlayerRespawn(respawn);
}

void Script::SetPlayerSpawnCell(NetworkID id, unsigned int cell)
{
	FactoryObject reference;

	try
	{
		reference = GameFactory::GetObject(id);
	}
	catch (...)
	{
		return;
	}

	Player* player = vaultcast<Player>(reference);

	if (player)
	{
		try
		{
			if (player->SetPlayerSpawnCell(cell))
			{
				NetworkResponse response;
				NetworkID id = player->GetNetworkID();
				RakNetGUID guid = Client::GetClientFromPlayer(id)->GetGUID();

				try
				{
					const Exterior& _cell = Exterior::Lookup(cell);

					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(id, _cell.GetWorld(), _cell.GetX(), _cell.GetY(), true),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
				}
				catch (...)
				{
					const Record& record = Record::Lookup(cell, "CELL");

					response.emplace_back(Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(id, record.GetName(), true),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
				}

				Network::Queue(move(response));
			}
		}
		catch (...) {}
	}
}
