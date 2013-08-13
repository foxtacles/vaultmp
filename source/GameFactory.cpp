#include "GameFactory.h"

using namespace std;
using namespace RakNet;

CriticalSection GameFactory::cs;
GameFactory::ReferenceList GameFactory::instances;
GameFactory::ReferenceIndex GameFactory::index;
GameFactory::ReferenceCount GameFactory::typecount;
GameFactory::ReferenceDeleted GameFactory::delrefs;
bool GameFactory::changed = false;

#ifdef VAULTMP_DEBUG
DebugInput<GameFactory> GameFactory::debug;
#endif

#ifdef VAULTSERVER
Database<DB::Record> GameFactory::dbRecords;
Database<DB::Reference> GameFactory::dbReferences;
Database<DB::Exterior> GameFactory::dbExteriors;
Database<DB::Weapon> GameFactory::dbWeapons;
Database<DB::Race> GameFactory::dbRaces;
Database<DB::NPC> GameFactory::dbNpcs;
Database<DB::BaseContainer> GameFactory::dbContainers;
Database<DB::Item> GameFactory::dbItems;
Database<DB::Terminal> GameFactory::dbTerminals;
Database<DB::Interior> GameFactory::dbInteriors;
#endif

void GameFactory::Initialize()
{
#ifdef VAULTSERVER
	dbRecords.initialize(DB_FALLOUT3, {"CONT", "NPC_", "CREA", "LVLI", "ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "ENCH", "KEYM", "MISC", "NOTE", "WEAP", "CELL", "IDLE", "WTHR", "STAT", "MSTT", "RACE", "LIGH", "DOOR", "TERM"});
	dbReferences.initialize(DB_FALLOUT3, {"refs_CONT", "refs_DOOR", "refs_TERM"});
	dbExteriors.initialize(DB_FALLOUT3, {"exteriors"});
	dbWeapons.initialize(DB_FALLOUT3, {"weapons"});
	dbRaces.initialize(DB_FALLOUT3, {"races"});
	dbNpcs.initialize(DB_FALLOUT3, {"npcs"});
	dbContainers.initialize(DB_FALLOUT3, {"npcitems", "contitems"});
	dbItems.initialize(DB_FALLOUT3, {"items"});
	dbTerminals.initialize(DB_FALLOUT3, {"terminals"});
	dbInteriors.initialize(DB_FALLOUT3, {"interiors"});
#endif
}

template<typename T>
vector<FactoryWrapper<T>> GameFactory::GetObjectTypes(unsigned int type) noexcept
{
	vector<FactoryWrapper<T>> result;
	ReferenceList::iterator it;

	cs.StartSession();

	result.reserve(typecount[type]);
	ReferenceList copy = instances;

	cs.EndSession();

	for (const auto& reference : copy)
		if (reference.second & type)
		{
			auto object = FactoryWrapper<T>(reference.first.get(), reference.second);

			if (object)
				result.emplace_back(move(object));
		}

	return result;
}
template vector<FactoryObject> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryItem> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryContainer> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryActor> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryPlayer> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryWindow> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryButton> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryText> GameFactory::GetObjectTypes(unsigned int type) noexcept;
template vector<FactoryEdit> GameFactory::GetObjectTypes(unsigned int type) noexcept;

vector<NetworkID> GameFactory::GetIDObjectTypes(unsigned int type) noexcept
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

unsigned int GameFactory::GetObjectCount(unsigned int type) noexcept
{
	cs.StartSession();

	unsigned int count = typecount[type];

	cs.EndSession();

	return count;
}

template<typename T>
Expected<FactoryWrapper<T>> GameFactory::GetObject(NetworkID id)
{
	pair<ReferenceList::key_type, ReferenceList::mapped_type> reference;

	cs.StartSession();

	auto it = GetShared(id);

	if (it != instances.end())
		reference = *it;

	cs.EndSession();

	if (!reference.first)
		return VaultException("Unknown object with NetworkID %llu", id);

	return FactoryWrapper<T>(reference.first.get(), reference.second);
}
template ExpectedObject GameFactory::GetObject(NetworkID id);
template ExpectedItem GameFactory::GetObject(NetworkID id);
template ExpectedContainer GameFactory::GetObject(NetworkID id);
template ExpectedActor GameFactory::GetObject(NetworkID id);
template ExpectedPlayer GameFactory::GetObject(NetworkID id);
template ExpectedWindow GameFactory::GetObject(NetworkID id);
template ExpectedButton GameFactory::GetObject(NetworkID id);
template ExpectedText GameFactory::GetObject(NetworkID id);
template ExpectedEdit GameFactory::GetObject(NetworkID id);

template<typename T>
Expected<FactoryWrapper<T>> GameFactory::GetObject(unsigned int refID)
{
	pair<ReferenceList::key_type, ReferenceList::mapped_type> reference;
	ReferenceList::iterator it;

	cs.StartSession();

	for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);

	if (it != instances.end())
		reference = *it;

	cs.EndSession();

	if (!reference.first)
		return VaultException("Unknown object with reference %08X", refID);

	return FactoryWrapper<T>(reference.first.get(), reference.second);
}
template ExpectedObject GameFactory::GetObject(unsigned int refID);
template ExpectedItem GameFactory::GetObject(unsigned int refID);
template ExpectedContainer GameFactory::GetObject(unsigned int refID);
template ExpectedActor GameFactory::GetObject(unsigned int refID);
template ExpectedPlayer GameFactory::GetObject(unsigned int refID);
template ExpectedWindow GameFactory::GetObject(unsigned int refID);
template ExpectedButton GameFactory::GetObject(unsigned int refID);
template ExpectedText GameFactory::GetObject(unsigned int refID);
template ExpectedEdit GameFactory::GetObject(unsigned int refID);

template<typename T>
vector<Expected<FactoryWrapper<T>>> GameFactory::GetMultiple(const vector<NetworkID>& objects)
{
	vector<Expected<FactoryWrapper<T>>> result(objects.size());
	multimap<ReferenceList::value_type, unsigned int> sort;

	cs.StartSession();

	unsigned int i = 0;

	for (const auto& id : objects)
	{
		auto it = GetShared(id);

		if (it == instances.end())
			result[i] = VaultException("Unknown object with NetworkID %llu", id);
		else
			// emplace
			sort.insert(make_pair(*it, i));

		++i;
	}

	cs.EndSession();

	for (const auto& reference : sort)
		result[reference.second] = FactoryWrapper<T>(reference.first.first.get(), reference.first.second);

	return result;
}
template vector<ExpectedObject> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedItem> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedContainer> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedActor> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedPlayer> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedWindow> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedButton> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedText> GameFactory::GetMultiple(const vector<NetworkID>& objects);
template vector<ExpectedEdit> GameFactory::GetMultiple(const vector<NetworkID>& objects);

template<typename T>
vector<Expected<FactoryWrapper<T>>> GameFactory::GetMultiple(const vector<unsigned int>& objects)
{
	vector<Expected<FactoryWrapper<T>>> result(objects.size());
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
		result[reference.second] = FactoryWrapper<T>(reference.first.first.get(), reference.first.second);

	return result;
}
template vector<ExpectedObject> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedItem> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedContainer> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedActor> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedPlayer> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedWindow> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedButton> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedText> GameFactory::GetMultiple(const vector<unsigned int>& objects);
template vector<ExpectedEdit> GameFactory::GetMultiple(const vector<unsigned int>& objects);

NetworkID GameFactory::LookupNetworkID(unsigned int refID)
{
	NetworkID id;
	ReferenceList::iterator it;

	cs.StartSession();

	for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);
	id = (it != instances.end() ? it->first->GetNetworkID() : 0);

	cs.EndSession();

	if (!id)
		throw VaultException("Unknown object with reference %08X", refID).stacktrace();

	return id;
}

unsigned int GameFactory::LookupRefID(NetworkID id)
{
	Reference* reference;
	unsigned int refID;

	cs.StartSession();

	auto it = GetShared(id);
	reference = it != instances.end() ? it->first.get() : nullptr;

	if (reference)
		refID = it->first->GetReference();

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
void GameFactory::LeaveReference(FactoryWrapper<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::LeaveReference Reference is NULL").stacktrace();

	_reference->EndSession();
	reference.reference = nullptr;
	reference.type = 0x00;
}
template void GameFactory::LeaveReference(FactoryObject& reference);
template void GameFactory::LeaveReference(FactoryItem& reference);
template void GameFactory::LeaveReference(FactoryContainer& reference);
template void GameFactory::LeaveReference(FactoryActor& reference);
template void GameFactory::LeaveReference(FactoryPlayer& reference);
template void GameFactory::LeaveReference(FactoryWindow& reference);
template void GameFactory::LeaveReference(FactoryButton& reference);
template void GameFactory::LeaveReference(FactoryText& reference);
template void GameFactory::LeaveReference(FactoryEdit& reference);

unsigned int GameFactory::GetType(const Reference* reference) noexcept
{
	return GetType(const_cast<Reference*>(reference)->GetNetworkID());
}

unsigned int GameFactory::GetType(NetworkID id) noexcept
{
	cs.StartSession();

	unsigned int type;
	auto it = GetShared(id);
	type = (it != instances.end() ? it->second : 0x00);

	cs.EndSession();

	return type;
}

unsigned int GameFactory::GetType(unsigned int refID) noexcept
{
	unsigned int type;

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

NetworkID GameFactory::CreateInstance(unsigned int type, unsigned int refID, unsigned int baseID)
{
	shared_ptr<Reference> reference;

	// can't use make_shared because of access control
	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance").stacktrace();

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

		case ID_WINDOW:
			reference = shared_ptr<Window>(new Window());
			break;

		case ID_BUTTON:
			reference = shared_ptr<Window>(new Button());
			break;

		case ID_TEXT:
			reference = shared_ptr<Window>(new Text());
			break;

		case ID_EDIT:
			reference = shared_ptr<Window>(new Edit());
			break;

		default:
			throw VaultException("Unknown type identifier %X", type).stacktrace();
	}

	NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->SetBase(reference->GetBase());
#endif

	cs.StartSession();

	++typecount[type];
	// emplace
	index[id] = instances.insert(make_pair(reference, type)).first;

	cs.EndSession();

	return id;
}

NetworkID GameFactory::CreateInstance(unsigned int type, unsigned int baseID)
{
	return CreateInstance(type, 0x00000000, baseID);
}

NetworkID GameFactory::CreateKnownInstance(unsigned int type, const pDefault* packet)
{
	shared_ptr<Reference> reference;

	// can't use make_shared because of access control
	switch (type)
	{
		case ID_REFERENCE:
			throw VaultException("It is not possible to have a pure Reference instance").stacktrace();

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

		case ID_WINDOW:
			reference = shared_ptr<Window>(new Window(packet));
			break;

		case ID_BUTTON:
			reference = shared_ptr<Window>(new Button(packet));
			break;

		case ID_TEXT:
			reference = shared_ptr<Window>(new Text(packet));
			break;

		case ID_EDIT:
			reference = shared_ptr<Window>(new Edit(packet));
			break;

		default:
			throw VaultException("Unknown type identifier %X", type).stacktrace();
	}

	NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->SetBase(reference->GetBase());
#endif

	reference->SetChanged(changed);

	cs.StartSession();

	++typecount[type];
	// emplace
	index[id] = instances.insert(make_pair(reference, type)).first;

	cs.EndSession();

	return id;
}

void GameFactory::DestroyAllInstances()
{
	cs.StartSession();

	for (const auto& instance : instances)
	{
		if (instance.second & ALL_CONTAINERS)
			static_cast<Container*>(instance.first.get())->IL.container.clear();

#ifdef VAULTMP_DEBUG
		debug.print("Reference ", hex, instance.first->GetReference(), " with base ", instance.first->GetBase(), " and NetworkID ", dec, instance.first->GetNetworkID(), " (type: ", typeid(*(instance.first)).name(), ") to be destructed (", instance.first.get(), ")");
#endif

		Reference* reference = static_cast<Reference*>(instance.first->StartSession());

		reference->Finalize();
	}

	instances.clear();
	index.clear();
	typecount.clear();
	delrefs.clear();

	cs.EndSession();

	// Cleanup classes

	Object::param_Axis = RawParameter(vector<string>());
	Actor::param_ActorValues = RawParameter(vector<string>());
	Window::childs.clear();

	Lockable::Reset();
}

bool GameFactory::DestroyInstance(NetworkID id)
{
	DestroyInstance(GetObject(id).get());
	return true;
}

template<typename T>
NetworkID GameFactory::DestroyInstance(FactoryWrapper<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::DestroyInstance Reference is NULL").stacktrace();

	NetworkID id = _reference->GetNetworkID();

#ifdef VAULTMP_DEBUG
	debug.print("Reference ", hex, _reference->GetReference(), " with base ",  _reference->GetBase(), " and NetworkID ", dec, _reference->GetNetworkID(), " (type: ", typeid(*_reference).name(), ") to be destructed");
#endif

	cs.StartSession();

	ReferenceList::iterator it = GetShared(id);

	--typecount[it->second];
	_reference->Finalize();
	instances.erase(it);
	index.erase(id);
	delrefs.emplace(id);

	cs.EndSession();

	reference.reference = nullptr;
	reference.type = 0x00;

	return id;
}
template NetworkID GameFactory::DestroyInstance(FactoryObject& reference);
template NetworkID GameFactory::DestroyInstance(FactoryItem& reference);
template NetworkID GameFactory::DestroyInstance(FactoryContainer& reference);
template NetworkID GameFactory::DestroyInstance(FactoryActor& reference);
template NetworkID GameFactory::DestroyInstance(FactoryPlayer& reference);
template NetworkID GameFactory::DestroyInstance(FactoryWindow& reference);
template NetworkID GameFactory::DestroyInstance(FactoryButton& reference);
template NetworkID GameFactory::DestroyInstance(FactoryText& reference);
template NetworkID GameFactory::DestroyInstance(FactoryEdit& reference);

void GameFactory::SetChangeFlag(bool changed)
{
	GameFactory::changed = changed;
}
