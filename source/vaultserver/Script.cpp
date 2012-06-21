#include "Script.h"

vector<Script*> Script::scripts;

Script::Script(char* path)
{
	FILE* file = fopen(path, "rb");

	if (file == NULL)
		throw VaultException("Script not found: %s", path);

	fclose(file);

	if (strstr(path, ".dll") || strstr(path, ".so"))
	{
		void* handle = NULL;
#ifdef __WIN32__
		handle = LoadLibrary(path);
#else
		handle = dlopen(path, RTLD_LAZY);
#endif

		if (handle == NULL)
			throw VaultException("Was not able to load C++ script: %s", path);

		this->handle = handle;
		this->cpp_script = true;
		scripts.push_back(this);

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
		SetScript(string(vpf + "IsValid").c_str(), &Script::IsValid);
		SetScript(string(vpf + "IsObject").c_str(), &Script::IsObject);
		SetScript(string(vpf + "IsItem").c_str(), &Script::IsItem);
		SetScript(string(vpf + "IsContainer").c_str(), &Script::IsContainer);
		SetScript(string(vpf + "IsActor").c_str(), &Script::IsActor);
		SetScript(string(vpf + "IsPlayer").c_str(), &Script::IsPlayer);
		SetScript(string(vpf + "GetType").c_str(), (unsigned char(*)(NetworkID)) &GameFactory::GetType);
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

		SetScript(string(vpf + "AddItem").c_str(), &Script::AddItem);
		SetScript(string(vpf + "RemoveItem").c_str(), &Script::RemoveItem);
		SetScript(string(vpf + "RemoveAllItems").c_str(), &Script::RemoveAllItems);
		SetScript(string(vpf + "SetActorValue").c_str(), &Script::SetActorValue);
		SetScript(string(vpf + "SetActorBaseValue").c_str(), &Script::SetActorBaseValue);
		SetScript(string(vpf + "EquipItem").c_str(), &Script::EquipItem);
		SetScript(string(vpf + "UnequipItem").c_str(), &Script::UnequipItem);
		SetScript(string(vpf + "KillActor").c_str(), &Script::KillActor);
		SetScript(string(vpf + "SetPlayerRespawn").c_str(), &Script::SetPlayerRespawn);

		fexec();
	}

	else if (strstr(path, ".amx"))
	{
		AMX* vaultscript = new AMX();

		this->handle = (void*) vaultscript;
		this->cpp_script = false;
		scripts.push_back(this);

		cell ret = 0;
		int err = 0;

		err = PAWN::LoadProgram(vaultscript, path, NULL);

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
		AMX* vaultscript = (AMX*) this->handle;
		PAWN::FreeProgram(vaultscript);
		delete vaultscript;
	}
}

void Script::LoadScripts(char* scripts, char* base)
{
	char* token = strtok(scripts, ",");

	try
	{
		while (token != NULL)
		{
#ifdef __WIN32__
			Script* script = new Script(token);
#else
			char path[MAX_PATH];
			snprintf(path, sizeof(path), "%s/%s", base, token);
			Script* script = new Script(path);
#endif
			token = strtok(NULL, ",");
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
	try
	{
		GameFactory::GetObject(id);
	}
	catch (...)
	{
		KillTimer();    // Player has already left the server
		return 0;
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::CreatePacket(ID_UPDATE_DEAD, id, false),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetClientFromPlayer(id)->GetGUID())
	});

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

void Script::OnActorDeath(FactoryObject& reference)
{
	NetworkID id = (*reference)->GetNetworkID();

	for (Script* script : scripts)
	{
		if (script->cpp_script)
		{
			if (script->fOnActorDeath)
				script->fOnActorDeath(id);
		}
		else if (PAWN::IsCallbackPresent((AMX*)script->handle, "OnActorDeath"))
			PAWN::Call((AMX*)script->handle, "OnActorDeath", "l", 0, id);
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
			if (!vaultcast<Player>(*GameFactory::GetObject(id)))
				return false;
		}
		catch (...)
		{
			return false;
		}
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::CreatePacket(ID_GAME_MESSAGE, _message.c_str()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(NULL))
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
			if (!vaultcast<Player>(*GameFactory::GetObject(id)))
				return false;
		}
		catch (...)
		{
			return false;
		}
	}

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::CreatePacket(ID_GAME_CHAT, _message.c_str()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, id ? vector<RakNetGUID>{Client::GetClientFromPlayer(id)->GetGUID()} : Client::GetNetworkList(NULL))
	});

	return true;
}

void Script::SetRespawn(unsigned int respawn)
{
	Player::SetRespawn(respawn);
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
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
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
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
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
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
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
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
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
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
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
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
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
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
			});

			actor->ApplyDiff(diff);

			return true;
		}
	}

	return false;
}

void Script::KillActor(NetworkID id)
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
				PacketFactory::CreatePacket(ID_UPDATE_DEAD, actor->GetNetworkID(), true),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL))
			});

			Script::OnActorDeath(reference);

			Player* player = vaultcast<Player>(actor);

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
