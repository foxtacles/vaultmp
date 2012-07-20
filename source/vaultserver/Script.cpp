#include "Script.h"

vector<Script*> Script::scripts;

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
			GetScript("OnPlayerDisconnect", fOnPlayerDisconnect);
			GetScript("OnPlayerRequestGame", fOnPlayerRequestGame);
			GetScript("OnPlayerChat", fOnPlayerChat);
			GetScript("OnClientAuthenticate", fOnClientAuthenticate);

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

			SetScript(string(vpf + "UIMessage").c_str(), &Script::UIMessage);
			SetScript(string(vpf + "ChatMessage").c_str(), &Script::ChatMessage);
			SetScript(string(vpf + "SetRespawn").c_str(), &Script::SetRespawn);
			SetScript(string(vpf + "SetSpawnCell").c_str(), &Script::SetSpawnCell);
			SetScript(string(vpf + "IsValid").c_str(), &Script::IsValid);
			SetScript(string(vpf + "IsObject").c_str(), &Script::IsObject);
			SetScript(string(vpf + "IsItem").c_str(), &Script::IsItem);
			SetScript(string(vpf + "IsContainer").c_str(), &Script::IsContainer);
			SetScript(string(vpf + "IsActor").c_str(), &Script::IsActor);
			SetScript(string(vpf + "IsPlayer").c_str(), &Script::IsPlayer);
			SetScript(string(vpf + "IsCell").c_str(), &Cell::IsValidCell);
			SetScript(string(vpf + "IsInterior").c_str(), &Script::IsInterior);
			SetScript(string(vpf + "GetType").c_str(), (unsigned char(*)(NetworkID)) &GameFactory::GetType);
			SetScript(string(vpf + "GetConnection").c_str(), &Script::GetConnection);
			SetScript(string(vpf + "GetCount").c_str(), &GameFactory::GetObjectCount);
			SetScript(string(vpf + "GetList").c_str(), &Script::GetList);

			SetScript(string(vpf + "GetReference").c_str(), &Script::GetReference);
			SetScript(string(vpf + "GetBase").c_str(), &Script::GetBase);
			SetScript(string(vpf + "GetName").c_str(), &Script::GetName);
			SetScript(string(vpf + "GetPos").c_str(), &Script::GetPos);
			SetScript(string(vpf + "GetAngle").c_str(), &Script::GetAngle);
			SetScript(string(vpf + "GetCell").c_str(), &Script::GetCell);
			SetScript(string(vpf + "IsNearPoint").c_str(), &Script::IsNearPoint);
			SetScript(string(vpf + "GetContainerItemCount").c_str(), &Script::GetContainerItemCount);
			SetScript(string(vpf + "GetActorValue").c_str(), &Script::GetActorValue);
			SetScript(string(vpf + "GetActorBaseValue").c_str(), &Script::GetActorBaseValue);
			SetScript(string(vpf + "GetActorMovingAnimation").c_str(), &Script::GetActorMovingAnimation);
			SetScript(string(vpf + "GetActorAlerted").c_str(), &Script::GetActorAlerted);
			SetScript(string(vpf + "GetActorSneaking").c_str(), &Script::GetActorSneaking);
			SetScript(string(vpf + "GetActorDead").c_str(), &Script::GetActorDead);
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
			SetScript(string(vpf + "KillActor").c_str(), &Script::KillActor);
			SetScript(string(vpf + "SetPlayerRespawn").c_str(), &Script::SetPlayerRespawn);
			SetScript(string(vpf + "SetPlayerSpawnCell").c_str(), &Script::SetPlayerSpawnCell);

			fexec();
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

			err = PAWN::Exec(vaultscript, &ret, AMX_EXEC_MAIN);

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
			scripts.push_back(script);
#endif
			token = strtok(nullptr, ",");
		}
	}
	catch (...)
	{
		UnloadScripts();
		throw;
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
					params.push_back(va_arg(args, unsigned int));
					break;
				}

				case 'l':
				{
					params.push_back(va_arg(args, unsigned long long));
					break;
				}

				case 'f':
				{
					params.push_back(va_arg(args, double));
					break;
				}

				case 'p':
				{
					params.push_back(va_arg(args, void*));
					break;
				}

				case 's':
				{
					params.push_back(string(va_arg(args, const char*)));
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
		KillTimer();    // Player has already left the server
		return 0;
	}

	Player* player = vaultcast<Player>(reference);
	RakNetGUID guid = Client::GetClientFromPlayer(id)->GetGUID();

	NetworkResponse response{Network::CreateResponse(
		PacketFactory::CreatePacket(ID_UPDATE_DEAD, id, false, 0, 0),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid)
	};

	unsigned int _cell = player->GetSpawnCell();

	try
	{
		const Cell& cell = Cell::Lookup(_cell);

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_EXTERIOR, static_cast<NetworkID>(0), cell.GetWorld(), cell.GetX(), cell.GetY()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}
	catch (...)
	{
		const Record& record = Record::Lookup(_cell);

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_INTERIOR, static_cast<NetworkID>(0), record.GetName().c_str()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	Network::Queue(move(response));

	KillTimer();

	return 1;
}

void Script::OnSpawn(FactoryObject& reference)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnCellChange(FactoryObject& reference, unsigned int cell)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnContainerItemChange(FactoryObject& reference, unsigned int baseID, signed int count, double condition)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnActorValueChange(FactoryObject& reference, unsigned char index, bool base, double value)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnActorAlert(FactoryObject& reference, bool alerted)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnActorSneak(FactoryObject& reference, bool sneaking)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnActorDeath(FactoryObject& reference, unsigned short limbs, signed char cause)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnActorEquipItem(FactoryObject& reference, unsigned int baseID, double condition)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnActorUnequipItem(FactoryObject& reference, unsigned int baseID, double condition)
{
	NetworkID id = (*reference)->GetNetworkID();

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

void Script::OnPlayerDisconnect(FactoryObject& reference, unsigned char reason)
{
	NetworkID id = (*reference)->GetNetworkID();

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

unsigned int Script::OnPlayerRequestGame(FactoryObject& reference)
{
	NetworkID id = (*reference)->GetNetworkID();
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

bool Script::OnPlayerChat(FactoryObject& reference, string& message)
{
	NetworkID id = (*reference)->GetNetworkID();
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
			result = (bool) PAWN::Call((AMX*)script->handle, "OnPlayerChat", "sl", 1, _message, id);
	}

	message.assign(_message);

	return result;
}

bool Script::OnClientAuthenticate(string name, string pwd)
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
			result = (bool) PAWN::Call((AMX*)script->handle, "OnClientAuthenticate", "ss", 0, pwd.c_str(), name.c_str());
	}

	return result;
}

const char* Script::ValueToString(unsigned char index)
{
	static string value;
	value = API::RetrieveValue_Reverse(index);
	return value.c_str();
}

const char* Script::AxisToString(unsigned char index)
{
	static string axis;
	axis = API::RetrieveAxis_Reverse(index);
	return axis.c_str();
}

const char* Script::AnimToString(unsigned char index)
{
	static string anim;
	anim = API::RetrieveAnim_Reverse(index);
	return anim.c_str();
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
		PacketFactory::CreatePacket(ID_GAME_MESSAGE, _message.c_str()),
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
		PacketFactory::CreatePacket(ID_GAME_CHAT, _message.c_str()),
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
	if (Cell::IsValidCell(cell))
		Player::SetSpawnCell(cell);
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
		const Record& record = Record::Lookup(cell);

		if (record.GetType().compare("CELL"))
			return false;
	}
	catch (...)
	{
		return false;
	}

	return true;
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
	*data = &_data.front();
	return _data.size();
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
		name = object->GetName();

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
	const Cell* new_cell = nullptr;

	try
	{
		try
		{
			const Record& record = Record::Lookup(cell);

			if (record.GetType().compare("CELL"))
				return state;
		}
		catch (...)
		{
			new_cell = &Cell::Lookup(Cell::Lookup(cell), X, Y);
		}
	}
	catch (...)
	{
		return state;
	}

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

				response.push_back(Network::CreateResponse(
					PacketFactory::CreatePacket(ID_UPDATE_CELL, id, new_cell),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				);

				if (player)
				{
					response.push_back(Network::CreateResponse(
						PacketFactory::CreatePacket(ID_UPDATE_EXTERIOR, id, new_cell->GetWorld(), new_cell->GetX(), new_cell->GetY()),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
					);
				}
			}
			else
				_new_cell = 0x00000000;
		}

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_POS, id, X, Y, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		);

		Network::Queue(move(response));

		state = true;
	}

	if (_new_cell)
		Script::OnCellChange(reference, _new_cell);

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
	const Cell* new_exterior = nullptr;

	try
	{
		try
		{
			new_interior = &Record::Lookup(cell);

			if (new_interior->GetType().compare("CELL"))
				return state;
		}
		catch (...)
		{
			if (update_pos)
			{
				new_exterior = &Cell::Lookup(Cell::Lookup(cell), X, Y);

				if (new_exterior->GetBase() != cell)
					throw VaultException("Coordinates (%f, %f, %f) not in cell %08X", X, Y, Z, cell);
			}
			else
				new_exterior = &Cell::Lookup(cell);
		}
	}
	catch (...)
	{
		return state;
	}

	NetworkResponse response;

	Player* player = vaultcast<Player>(reference);

	if (object->SetNetworkCell(cell))
	{
		object->SetGameCell(cell);

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_CELL, id, cell),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		);

		if (player)
		{
			if (new_interior)
			{
				response.push_back(Network::CreateResponse(
					PacketFactory::CreatePacket(ID_UPDATE_INTERIOR, id, new_interior->GetName().c_str()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
				);
			}
			else
			{
				response.push_back(Network::CreateResponse(
					PacketFactory::CreatePacket(ID_UPDATE_EXTERIOR, id, new_exterior->GetWorld(), new_exterior->GetX(), new_exterior->GetY()),
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

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_POS, id, X, Y, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
		);

		state = true;
	}

	if (state)
		Network::Queue(move(response));

	if (cell)
		Script::OnCellChange(reference, cell);

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
					PacketFactory::CreatePacket(ID_UPDATE_CONTAINER, id, &diff),
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
						PacketFactory::CreatePacket(ID_UPDATE_CONTAINER, id, &diff),
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
					PacketFactory::CreatePacket(ID_UPDATE_CONTAINER, id, &diff),
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
					PacketFactory::CreatePacket(ID_UPDATE_VALUE, actor->GetNetworkID(), false, index, value),
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
		FactoryObject reference = GameFactory::GetObject(id);
		Actor* actor = vaultcast<Actor>(reference);

		if (actor)
		{
			if (actor->SetActorBaseValue(index, value))
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::CreatePacket(ID_UPDATE_VALUE, actor->GetNetworkID(), true, index, value),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
				});
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
				PacketFactory::CreatePacket(ID_UPDATE_CONTAINER, id, &diff),
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
				PacketFactory::CreatePacket(ID_UPDATE_CONTAINER, id, &diff),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			actor->ApplyDiff(diff);

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
				PacketFactory::CreatePacket(ID_UPDATE_DEAD, actor->GetNetworkID(), true, limbs, cause),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
			});

			Script::OnActorDeath(reference, limbs, cause);

			Player* player = vaultcast<Player>(reference);

			if (player)
				Script::CreateTimerEx(reinterpret_cast<ScriptFunc>(&Script::Timer_Respawn), player->GetPlayerRespawn(), "l", player->GetNetworkID());
		}
	}
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

	if (player && Cell::IsValidCell(cell))
		player->SetPlayerSpawnCell(cell);
}
