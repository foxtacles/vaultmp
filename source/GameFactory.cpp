#include "GameFactory.h"

using namespace std;
using namespace RakNet;

CriticalSection GameFactory::cs;
ReferenceList GameFactory::instances;
ReferenceCount GameFactory::typecount;
ReferenceDeleted GameFactory::delrefs;
unsigned char GameFactory::game = 0x00;
bool GameFactory::changed = false;

#ifdef VAULTMP_DEBUG
DebugInput<GameFactory> GameFactory::debug;
#endif

#ifdef VAULTSERVER
Database<DB::Record> GameFactory::dbRecords;
Database<DB::Exterior> GameFactory::dbExteriors;
Database<DB::Weapon> GameFactory::dbWeapons;
Database<DB::Race> GameFactory::dbRaces;
Database<DB::NPC> GameFactory::dbNpcs;
Database<DB::BaseContainer> GameFactory::dbContainers;
Database<DB::Item> GameFactory::dbItems;
#endif

void GameFactory::Initialize(unsigned char game)
{
	GameFactory::game = game;

#ifdef VAULTSERVER
	switch (game)
	{
		case FALLOUT3:
			dbRecords.initialize(DB_FALLOUT3, {"CONT", "NPC_", "CREA", "LVLI", "ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "ENCH", "KEYM", "MISC", "NOTE", "WEAP", "CELL", "IDLE", "WTHR", "STAT", "MSTT", "RACE", "LIGH"});
			dbExteriors.initialize(DB_FALLOUT3, {"exteriors"});
			dbWeapons.initialize(DB_FALLOUT3, {"weapons"});
			dbRaces.initialize(DB_FALLOUT3, {"races"});
			dbNpcs.initialize(DB_FALLOUT3, {"npcs"});
			dbContainers.initialize(DB_FALLOUT3, {"npcitems", "contitems"});
			dbItems.initialize(DB_FALLOUT3, {"items"});
			break;

		case NEWVEGAS:
			dbRecords.initialize(DB_NEWVEGAS, {"CONT", "NPC_", "CREA", "LVLI", "ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "CCRD", "CDCK", "CHIP", "CMNY", "ENCH", "IMOD", "KEYM", "MISC", "NOTE", "RCPE", "WEAP", "CELL", "IDLE", "WTHR", "STAT", "MSTT", "RACE", "LIGH"});
			dbExteriors.initialize(DB_NEWVEGAS, {"exteriors"});
			dbWeapons.initialize(DB_NEWVEGAS, {"weapons"});
			dbRaces.initialize(DB_NEWVEGAS, {"races"});
			dbNpcs.initialize(DB_NEWVEGAS, {"npcs"});
			dbContainers.initialize(DB_NEWVEGAS, {"npcitems", "contitems"});
			dbItems.initialize(DB_NEWVEGAS, {"items"});
			break;

		default:
			throw VaultException("Bad game ID %08X", game);
	}
#endif
}

template<typename T>
vector<FactoryObject<T>> GameFactory::GetObjectTypes(unsigned char type) noexcept
{
	vector<FactoryObject<T>> result;
	ReferenceList::iterator it;

	cs.StartSession();

	result.reserve(typecount[type]);
	ReferenceList copy = instances;

	cs.EndSession();

	for (const auto& reference : copy)
		if (reference.second & type)
		{
			auto object = FactoryObject<T>(reference.first.get(), reference.second);

			if (object)
				result.emplace_back(std::move(object));
		}

	return result;
}
template vector<FactoryObject<Object>> GameFactory::GetObjectTypes(unsigned char type) noexcept;
template vector<FactoryObject<Item>> GameFactory::GetObjectTypes(unsigned char type) noexcept;
template vector<FactoryObject<Container>> GameFactory::GetObjectTypes(unsigned char type) noexcept;
template vector<FactoryObject<Actor>> GameFactory::GetObjectTypes(unsigned char type) noexcept;
template vector<FactoryObject<Player>> GameFactory::GetObjectTypes(unsigned char type) noexcept;

vector<NetworkID> GameFactory::GetIDObjectTypes(unsigned char type) noexcept
{
	vector<NetworkID> result;
	ReferenceList::iterator it;

	cs.StartSession();

	result.reserve(typecount[type]);
	ReferenceList copy = instances;

	cs.EndSession();

	for (const auto& reference : copy)
		if (reference.second & type)
			result.emplace_back(reference.first->GetNetworkID());

	return result;
}

unsigned int GameFactory::GetObjectCount(unsigned char type) noexcept
{
	cs.StartSession();

	unsigned int count = typecount[type];

	cs.EndSession();

	return count;
}

template<typename T>
Expected<FactoryObject<T>> GameFactory::GetObject(NetworkID id)
{
	Reference* reference;
	unsigned char type;

	cs.StartSession();

	reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);

	if (reference)
		type = GetShared(reference)->second;

	cs.EndSession();

	if (!reference)
		return VaultException("Unknown object with NetworkID %llu", id);

	return FactoryObject<T>(reference, type);
}
template Expected<FactoryObject<Object>> GameFactory::GetObject(NetworkID id);
template Expected<FactoryObject<Item>> GameFactory::GetObject(NetworkID id);
template Expected<FactoryObject<Container>> GameFactory::GetObject(NetworkID id);
template Expected<FactoryObject<Actor>> GameFactory::GetObject(NetworkID id);
template Expected<FactoryObject<Player>> GameFactory::GetObject(NetworkID id);

template<typename T>
Expected<FactoryObject<T>> GameFactory::GetObject(unsigned int refID)
{
	Reference* reference;
	unsigned char type;
	ReferenceList::iterator it;

	cs.StartSession();

	for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);

	if (it != instances.end())
	{
		reference = it->first.get();
		type = it->second;
	}
	else
		reference = nullptr;

	cs.EndSession();

	if (!reference)
		return VaultException("Unknown object with reference %08X", refID);

	return FactoryObject<T>(reference, type);
}
template Expected<FactoryObject<Object>> GameFactory::GetObject(unsigned int refID);
template Expected<FactoryObject<Item>> GameFactory::GetObject(unsigned int refID);
template Expected<FactoryObject<Container>> GameFactory::GetObject(unsigned int refID);
template Expected<FactoryObject<Actor>> GameFactory::GetObject(unsigned int refID);
template Expected<FactoryObject<Player>> GameFactory::GetObject(unsigned int refID);

template<typename T>
vector<Expected<FactoryObject<T>>> GameFactory::GetMultiple(const vector<NetworkID>& objects)
{
	vector<Expected<FactoryObject<T>>> result(objects.size());
	multimap<ReferenceList::value_type, unsigned int> sort;

	cs.StartSession();

	unsigned int i = 0;

	for (const auto& id : objects)
	{
		Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);

		if (!reference)
			result[i] = VaultException("Unknown object with NetworkID %llu", id);
		else
			// emplace
			sort.insert(make_pair(*GetShared(reference), i));

		++i;
	}

	cs.EndSession();

	for (const auto& reference : sort)
		result[reference.second] = FactoryObject<T>(reference.first.first.get(), reference.first.second);

	return result;
}
template vector<Expected<FactoryObject<Object>>> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<Expected<FactoryObject<Item>>> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<Expected<FactoryObject<Container>>> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<Expected<FactoryObject<Actor>>> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<Expected<FactoryObject<Player>>> GameFactory::GetMultiple(const vector<NetworkID>& objects);

template<typename T>
vector<Expected<FactoryObject<T>>> GameFactory::GetMultiple(const vector<unsigned int>& objects)
{
	vector<Expected<FactoryObject<T>>> result(objects.size());
	multimap<ReferenceList::value_type, unsigned int> sort;

	cs.StartSession();

	ReferenceList::iterator it;
	unsigned int i = 0;

	for (const auto& refID : objects)
	{
		for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);

		if (it == instances.end())
			result[i] = VaultException("Unknown object with reference %08X", refID);
		else
			// emplace
			sort.insert(make_pair(*it, i));

		++i;
	}

	cs.EndSession();

	for (const auto& reference : sort)
		result[reference.second] = FactoryObject<T>(reference.first.first.get(), reference.first.second);

	return result;
}
template vector<Expected<FactoryObject<Object>>> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<Expected<FactoryObject<Item>>> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<Expected<FactoryObject<Container>>> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<Expected<FactoryObject<Actor>>> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<Expected<FactoryObject<Player>>> GameFactory::GetMultiple(const vector<unsigned int>& objects);

NetworkID GameFactory::LookupNetworkID(unsigned int refID)
{
	NetworkID id;
	ReferenceList::iterator it;

	cs.StartSession();

	for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);
	id = (it != instances.end() ? it->first->GetNetworkID() : 0);

	cs.EndSession();

	if (!id)
		throw VaultException("Unknown object with reference %08X", refID);

	return id;
}

unsigned int GameFactory::LookupRefID(NetworkID id)
{
	Reference* reference;
	unsigned int refID;

	cs.StartSession();

	reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);

	if (reference)
		refID = reference->GetReference();

	cs.EndSession();

	if (!reference)
		throw VaultException("Unknown object with NetworkID %llu", id);

	return refID;
}

bool GameFactory::IsDeleted(NetworkID id)
{
	bool result;

	cs.StartSession();

	result = delrefs.find(id) != delrefs.end();

	cs.EndSession();

	return result;
}

template<typename T>
void GameFactory::LeaveReference(FactoryObject<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::LeaveReference Reference is NULL");

	_reference->EndSession();
	reference.reference = nullptr;
	reference.type = 0x00;
}
template void GameFactory::LeaveReference(FactoryObject<Object>& reference);
template void GameFactory::LeaveReference(FactoryObject<Item>& reference);
template void GameFactory::LeaveReference(FactoryObject<Container>& reference);
template void GameFactory::LeaveReference(FactoryObject<Actor>& reference);
template void GameFactory::LeaveReference(FactoryObject<Player>& reference);

unsigned char GameFactory::GetType(const Reference* reference) noexcept
{
	ReferenceList::iterator it;

	cs.StartSession();

	unsigned char type;
	it = GetShared(reference);
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
	shared_ptr<Reference> reference;

	// can't use make_shared because of access control
	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance");

		case ID_OBJECT:
			reference = shared_ptr<Object>(new Object(refID, baseID));
			break;

		case ID_ITEM:
			reference = shared_ptr<Item>(new Item(refID, baseID));
			break;

		case ID_CONTAINER:
			reference = shared_ptr<Container>(new Container(refID, baseID));
			break;

		case ID_ACTOR:
			reference = shared_ptr<Actor>(new Actor(refID, baseID));
			break;

		case ID_PLAYER:
			reference = shared_ptr<Player>(new Player(refID, baseID));
			break;

		default:
			throw VaultException("Unknown type identifier %X", type);
	}

	NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->SetBase(reference->GetBase());
#endif

	cs.StartSession();

	++typecount[type];
	// emplace
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
	shared_ptr<Reference> reference;

	// can't use make_shared because of access control
	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance");

		case ID_OBJECT:
			reference = shared_ptr<Object>(new Object(refID, baseID));
			break;

		case ID_ITEM:
			reference = shared_ptr<Item>(new Item(refID, baseID));
			break;

		case ID_CONTAINER:
			reference = shared_ptr<Container>(new Container(refID, baseID));
			break;

		case ID_ACTOR:
			reference = shared_ptr<Actor>(new Actor(refID, baseID));
			break;

		case ID_PLAYER:
			reference = shared_ptr<Player>(new Player(refID, baseID));
			break;

		default:
			throw VaultException("Unknown type identifier %X", type);
	}

	reference->SetNetworkID(id);

#ifdef VAULTSERVER
	reference->SetBase(reference->GetBase());
#endif

	cs.StartSession();

	++typecount[type];
	// emplace
	instances.insert(make_pair(reference, type));

	cs.EndSession();
}

void GameFactory::CreateKnownInstance(unsigned char type, NetworkID id, unsigned int baseID)
{
	return CreateKnownInstance(type, id, 0x00, baseID);
}

NetworkID GameFactory::CreateKnownInstance(unsigned char type, const pDefault* packet)
{
	shared_ptr<Reference> reference;

	// can't use make_shared because of access control
	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance");

		case ID_OBJECT:
			reference = shared_ptr<Object>(new Object(packet));
			break;

		case ID_ITEM:
			reference = shared_ptr<Item>(new Item(packet));
			break;

		case ID_CONTAINER:
			reference = shared_ptr<Container>(new Container(packet));
			break;

		case ID_ACTOR:
			reference = shared_ptr<Actor>(new Actor(packet));
			break;

		case ID_PLAYER:
			reference = shared_ptr<Player>(new Player(packet));
			break;

		default:
			throw VaultException("Unknown type identifier %X", type);
	}

	NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->SetBase(reference->GetBase());
#endif

	reference->SetChanged(changed);

	cs.StartSession();

	++typecount[type];
	// emplace
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
			static_cast<Container*>(instance.first.get())->container.clear();

#ifdef VAULTMP_DEBUG
		debug.print("Reference ", hex, instance.first->GetReference(), " with base ", instance.first->GetBase(), " and NetworkID ", dec, instance.first->GetNetworkID(), " (type: ", typeid(*(instance.first)).name(), ") to be destructed (", instance.first.get(), ")");
#endif

		Reference* reference = static_cast<Reference*>(instance.first->StartSession());

		reference->Finalize();
	}

	instances.clear();
	typecount.clear();
	delrefs.clear();

	cs.EndSession();

	// Cleanup classes

	Object::param_Axis = RawParameter(vector<string>());
	Actor::param_ActorValues = RawParameter(vector<string>());

	Lockable::Reset();
}

bool GameFactory::DestroyInstance(NetworkID id)
{
	DestroyInstance(GetObject(id).get());
	return true;
}

template<typename T>
NetworkID GameFactory::DestroyInstance(FactoryObject<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::DestroyInstance Reference is NULL");

	NetworkID id = _reference->GetNetworkID();

#ifdef VAULTMP_DEBUG
	debug.print("Reference ", hex, _reference->GetReference(), " with base ",  _reference->GetBase(), " and NetworkID ", dec, _reference->GetNetworkID(), " (type: ", typeid(*_reference).name(), ") to be destructed");
#endif

	cs.StartSession();

	ReferenceList::iterator it = GetShared(_reference);

	--typecount[it->second];
	_reference->Finalize();
	instances.erase(it);
	delrefs.emplace(id);

	cs.EndSession();

	reference.reference = nullptr;
	reference.type = 0x00;

	return id;
}
template NetworkID GameFactory::DestroyInstance(FactoryObject<Object>& reference);
template NetworkID GameFactory::DestroyInstance(FactoryObject<Item>& reference);
template NetworkID GameFactory::DestroyInstance(FactoryObject<Container>& reference);
template NetworkID GameFactory::DestroyInstance(FactoryObject<Actor>& reference);
template NetworkID GameFactory::DestroyInstance(FactoryObject<Player>& reference);

void GameFactory::SetChangeFlag(bool changed)
{
	GameFactory::changed = changed;
}
