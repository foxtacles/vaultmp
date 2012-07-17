#include "GameFactory.h"

CriticalSection GameFactory::cs;
ReferenceList GameFactory::instances;
ReferenceCount GameFactory::typecount;
unsigned char GameFactory::game = 0x00;

#ifdef VAULTMP_DEBUG
Debug* GameFactory::debug;
#endif

#ifdef VAULTSERVER
Database<Record> GameFactory::dbRecords;
Database<Cell> GameFactory::dbCells;
#endif

void GameFactory::Initialize(unsigned char game)
{
	GameFactory::game = game;

#ifdef VAULTSERVER
	switch (game)
	{
		case FALLOUT3:
			dbRecords.initialize(DB_FALLOUT3, {"NPC_", "CREA", "ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "ENCH", "KEYM", "MISC", "NOTE", "PROJ", "WEAP", "CELL"});
			dbCells.initialize(DB_FALLOUT3, {"exteriors"});
			break;

		case NEWVEGAS:
			dbRecords.initialize(DB_NEWVEGAS, {"NPC_", "CREA", "ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "CCRD", "CDCK", "CHIP", "CMNY", "ENCH", "IMOD", "KEYM", "MISC", "NOTE", "PROJ", "RCPE", "WEAP", "CELL"});
			dbCells.initialize(DB_NEWVEGAS, {"exteriors"});
			break;

		default:
			throw VaultException("Bad game ID %08X", game);
	}
#endif
}

#ifdef VAULTMP_DEBUG
void GameFactory::SetDebugHandler(Debug* debug)
{
	GameFactory::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to GameFactory class", true);
}
#endif

vector<FactoryObject> GameFactory::GetObjectTypes(unsigned char type) noexcept
{
	vector<FactoryObject> result;
	ReferenceList::iterator it;

	cs.StartSession();

	result.reserve(typecount[type]);
	ReferenceList copy = instances;

	cs.EndSession();

	for (it = copy.begin(); it != copy.end(); ++it)
		if (it->second & type)
			result.push_back(FactoryObject(it->first, type));

	return result;
}

vector<NetworkID> GameFactory::GetIDObjectTypes(unsigned char type) noexcept
{
	vector<NetworkID> result;
	ReferenceList::iterator it;

	cs.StartSession();

	result.reserve(typecount[type]);
	ReferenceList copy = instances;

	cs.EndSession();

	for (it = copy.begin(); it != copy.end(); ++it)
		if (it->second & type)
			result.push_back(it->first->GetNetworkID());

	return result;
}

unsigned int GameFactory::GetObjectCount(unsigned char type) noexcept
{
	cs.StartSession();

	unsigned int count = typecount[type];

	cs.EndSession();

	return count;
}

FactoryObject GameFactory::GetObject(NetworkID id)
{
	cs.StartSession();

	Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);
	unsigned char type = instances.find(reference)->second;

	cs.EndSession();

	if (!reference)
		throw VaultException("Unknown object with NetworkID %lld", id);

	return FactoryObject(reference, type);
}

FactoryObject GameFactory::GetObject(unsigned int refID)
{
	ReferenceList::iterator it;

	cs.StartSession();

	for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);

	Reference* reference;
	unsigned char type;

	if (it != instances.end())
	{
		reference = it->first;
		type = it->second;
	}
	else
	{
		reference = NULL;
		type = 0x00;
	}

	cs.EndSession();

	if (!reference)
		throw VaultException("Unknown object with reference %08X", refID);

	return FactoryObject(reference, type);
}

vector<FactoryObject> GameFactory::GetMultiple(const vector<NetworkID>& objects)
{
	vector<FactoryObject> result(objects.size());
	multimap<pair<Reference*, unsigned char>, unsigned int> sort;

	cs.StartSession();

	try
	{
		unsigned int i = 0;

		for (const auto& id : objects)
		{
			Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);

			if (!reference)
				throw VaultException("Unknown object with NetworkID %lld", id);

			sort.insert(make_pair(*instances.find(reference), i));

			++i;
		}
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

	cs.EndSession();

	for (const auto& reference : sort)
		result[reference.second] = FactoryObject(reference.first.first, reference.first.second);

	return result;
}

vector<FactoryObject> GameFactory::GetMultiple(const vector<unsigned int>& objects)
{
	vector<FactoryObject> result(objects.size());
	multimap<pair<Reference*, unsigned char>, unsigned int> sort;

	cs.StartSession();

	try
	{
		ReferenceList::iterator it;
		unsigned int i = 0;

		for (const auto& id : objects)
		{
			for (it = instances.begin(); it != instances.end() && it->first->GetReference() != id; ++it);

			if (it == instances.end())
				throw VaultException("Unknown object with reference %08X", id);

			sort.insert(make_pair(*it, i));

			++i;
		}
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

	cs.EndSession();

	for (const auto& reference : sort)
		result[reference.second] = FactoryObject(reference.first.first, reference.first.second);

	return result;
}

NetworkID GameFactory::LookupNetworkID(unsigned int refID)
{
	NetworkID id;

	cs.StartSession();

	try
	{
		ReferenceList::iterator it;

		for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);
		id = (it != instances.end() ? it->first->GetNetworkID() : throw VaultException("Unknown object with reference %08X", refID));
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

	cs.EndSession();

	return id;
}

unsigned int GameFactory::LookupRefID(NetworkID id)
{
	unsigned int refID;

	cs.StartSession();

	try
	{
		Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);
		refID = (reference != NULL ? reference->GetReference() : throw VaultException("Unknown object with NetworkID %lld", id));
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

	cs.EndSession();

	return refID;
}

void GameFactory::LeaveReference(FactoryObject& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::LeaveReference Reference is NULL");

	_reference->EndSession();
	reference.reference = NULL;
	reference.type = 0x00;
}

unsigned char GameFactory::GetType(Reference* reference) noexcept
{
	ReferenceList::iterator it;

	cs.StartSession();

	unsigned char type;
	it = instances.find(reference);
	type = (it != instances.end() ? it->second : 0x00);

	cs.EndSession();

	return type;
}

unsigned char GameFactory::GetType(NetworkID id) noexcept
{
	cs.StartSession();

	Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);
	unsigned char type = GetType(reference);

	cs.EndSession();

	return type;
}

unsigned char GameFactory::GetType(unsigned int refID) noexcept
{
	unsigned char type;

	cs.StartSession();

	try
	{
		type = GetType(LookupNetworkID(refID));
	}
	catch (...)
	{
		type = 0x00;
	}

	cs.EndSession();

	return type;
}

NetworkID GameFactory::CreateInstance(unsigned char type, unsigned int refID, unsigned int baseID)
{
	Reference* reference;

	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance");

		case ID_OBJECT:
			reference = new Object(refID, baseID);
			break;

		case ID_ITEM:
			reference = new Item(refID, baseID);
			break;

		case ID_CONTAINER:
			reference = new Container(refID, baseID);
			break;

		case ID_ACTOR:
			reference = new Actor(refID, baseID);
			break;

		case ID_PLAYER:
			reference = new Player(refID, baseID);
			break;

		default:
			throw VaultException("Unknown type identifier %X", type);
	}

	NetworkID id = reference->GetNetworkID();

	cs.StartSession();

	++typecount[type];
	instances.insert(make_pair(reference, type));

	cs.EndSession();

	return id;
}

NetworkID GameFactory::CreateInstance(unsigned char type, unsigned int baseID)
{
	return CreateInstance(type, 0x00, baseID);
}

void GameFactory::CreateKnownInstance(unsigned char type, NetworkID id, unsigned int refID, unsigned int baseID)
{
	Reference* reference;

	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance");

		case ID_OBJECT:
			reference = new Object(refID, baseID);
			break;

		case ID_ITEM:
			reference = new Item(refID, baseID);
			break;

		case ID_CONTAINER:
			reference = new Container(refID, baseID);
			break;

		case ID_ACTOR:
			reference = new Actor(refID, baseID);
			break;

		case ID_PLAYER:
			reference = new Player(refID, baseID);
			break;

		default:
			throw VaultException("Unknown type identifier %X", type);
	}

	reference->SetNetworkID(id);

	cs.StartSession();

	++typecount[type];
	instances.insert(make_pair(reference, type));

	cs.EndSession();
}

void GameFactory::CreateKnownInstance(unsigned char type, NetworkID id, unsigned int baseID)
{
	return CreateKnownInstance(type, id, 0x00, baseID);
}

NetworkID GameFactory::CreateKnownInstance(unsigned char type, const pDefault* packet)
{
	Reference* reference;

	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance");

		case ID_OBJECT:
			reference = new Object(packet);
			break;

		case ID_ITEM:
			reference = new Item(packet);
			break;

		case ID_CONTAINER:
			reference = new Container(packet);
			break;

		case ID_ACTOR:
			reference = new Actor(packet);
			break;

		case ID_PLAYER:
			reference = new Player(packet);
			break;

		default:
			throw VaultException("Unknown type identifier %X", type);
	}

	NetworkID id = PacketFactory::ExtractNetworkID(packet);
	reference->SetNetworkID(id);

	cs.StartSession();

	++typecount[type];
	instances.insert(make_pair(reference, type));

	cs.EndSession();

	return id;
}

void GameFactory::DestroyAllInstances()
{
	cs.StartSession();

	for (const auto& instance : instances)
	{
		if (instance.second & ALL_CONTAINERS)
			reinterpret_cast<Container*>(instance.first)->container.clear();

#ifdef VAULTMP_DEBUG

		if (debug)
			debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed (%08X)", true, instance.first->GetReference(), instance.first->GetBase(), instance.first->GetNetworkID(), typeid(*(instance.first)).name(), instance.first);

#endif

		Reference* reference = reinterpret_cast<Reference*>(instance.first->StartSession());

		if (reference)
		{
			reference->Finalize();
			delete reference; // this throws
		}
	}

	instances.clear();
	typecount.clear();

	cs.EndSession();

	// Cleanup classes

	Object::param_Axis = RawParameter(vector<string>());
	Actor::param_ActorValues = RawParameter(vector<string>());

	Lockable::Reset();
}

bool GameFactory::DestroyInstance(NetworkID id)
{
	FactoryObject reference = GetObject(id);
	DestroyInstance(reference);
	return true;
}

NetworkID GameFactory::DestroyInstance(FactoryObject& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::DestroyInstance Reference is NULL");

	NetworkID id = _reference->GetNetworkID();

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed", true, _reference->GetReference(), _reference->GetBase(), _reference->GetNetworkID(), typeid(*_reference).name());

#endif

	cs.StartSession();

	ReferenceList::iterator it = instances.find(_reference);

	if (it != instances.end())
	{
		--typecount[it->second];
		instances.erase(it);
	}

	cs.EndSession();

	_reference->Finalize();
	delete _reference; // this throws
	reference.reference = NULL;
	reference.type = 0x00;

	return id;
}
