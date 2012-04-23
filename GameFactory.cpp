#include "GameFactory.h"

CriticalSection GameFactory::cs;
ReferenceList GameFactory::instances;
unsigned char GameFactory::game = 0x00;

#ifdef VAULTMP_DEBUG
Debug* GameFactory::debug;
#endif

void GameFactory::Initialize(unsigned char game)
{
	GameFactory::game = game;

	switch (game)
	{
		case FALLOUT3:
			Container::Items = &Container::Fallout3Items;
			break;

		case NEWVEGAS:
			Container::Items = &Container::FalloutNVItems;
			break;

		default:
			throw VaultException("Bad game ID %08X", game);
	}

#ifdef VAULTMP_DEBUG

	if (debug != NULL)
		debug->PrintFormat("Found %d items in the database", true, Container::Items->size());

#endif
}

#ifdef VAULTMP_DEBUG
void GameFactory::SetDebugHandler(Debug* debug)
{
	GameFactory::debug = debug;

	if (debug != NULL)
		debug->Print("Attached debug handler to GameFactory class", true);
}
#endif

vector<FactoryObject> GameFactory::GetObjectTypes(unsigned char type)
{
	vector<FactoryObject> result;
	ReferenceList::iterator it;

	cs.StartSession();

	ReferenceList copy = instances;

	cs.EndSession();

	for (it = copy.begin(); it != copy.end() && (it->second & type); ++it)
		result.push_back(FactoryObject(it->first));

	return result;
}

vector<NetworkID> GameFactory::GetIDObjectTypes(unsigned char type)
{
	vector<NetworkID> result;
	ReferenceList::iterator it;

	cs.StartSession();

	ReferenceList copy = instances;

	cs.EndSession();

	for (it = copy.begin(); it != copy.end() && (it->second & type); ++it)
		result.push_back(it->first->GetNetworkID());

	return result;
}

FactoryObject GameFactory::GetObject(NetworkID id)
{
	cs.StartSession();

	Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);

	cs.EndSession();

	if (!reference)
		throw VaultException("Unknown object with NetworkID %lld", id);

	return FactoryObject(reference);
}

FactoryObject GameFactory::GetObject(unsigned int refID)
{
	ReferenceList::iterator it;

	cs.StartSession();

	for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);

	Reference* reference = (it != instances.end() ? it->first : NULL);

	cs.EndSession();

	if (!reference)
		throw VaultException("Unknown object with reference %08X", refID);

	return FactoryObject(reference);
}

vector<FactoryObject> GameFactory::GetMultiple(const vector<NetworkID>& objects)
{
	vector<FactoryObject> result;
	result.resize(objects.size(), FactoryObject());
	multimap<Reference*, unsigned int> sort;
	unsigned int i = 0;

	cs.StartSession();

	try
	{
	for (const NetworkID & id : objects)
		{
			Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);

			if (!reference)
				throw VaultException("Unknown object with NetworkID %lld", id);

			sort.insert(pair<Reference*, unsigned int>(reference, i));

			++i;
		}
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

	cs.EndSession();

for (pair<Reference * const, unsigned int>& reference : sort)
		result[reference.second] = FactoryObject(reference.first);

	return result;
}

vector<FactoryObject> GameFactory::GetMultiple(const vector<unsigned int>& objects)
{
	vector<FactoryObject> result;
	result.resize(objects.size(), FactoryObject());
	multimap<Reference*, unsigned int> sort;
	ReferenceList::iterator it;
	unsigned int i = 0;

	cs.StartSession();

	try
	{
	for (const NetworkID & id : objects)
		{
			for (it = instances.begin(); it != instances.end() && it->first->GetReference() != id; ++it);

			Reference* reference = (it != instances.end() ? it->first : NULL);

			if (!reference)
				throw VaultException("Unknown object with reference %08X", id);

			sort.insert(pair<Reference*, unsigned int>(reference, i));

			++i;
		}
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

	cs.EndSession();

for (pair<Reference * const, unsigned int>& reference : sort)
		result[reference.second] = FactoryObject(reference.first);

	return result;
}

NetworkID GameFactory::LookupNetworkID(unsigned int refID)
{
	ReferenceList::iterator it;

	cs.StartSession();

	NetworkID id;

	try
	{
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
	cs.StartSession();

	unsigned int refID;

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
	cs.StartSession();

	unsigned char type;

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

	instances.insert(pair<Reference*, unsigned char>(reference, type));

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

	instances.insert(pair<Reference*, unsigned char>(reference, type));

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

	instances.insert(pair<Reference*, unsigned char>(reference, type));

	cs.EndSession();

	return id;
}

void GameFactory::DestroyAllInstances()
{
	cs.StartSession();

for (pair<Reference * const, unsigned char>& instance : instances)
	{
		if (instance.second & ALL_CONTAINERS)
			vaultcast<Container>(instance.first)->container.clear();

#ifdef VAULTMP_DEBUG

		if (debug != NULL)
			debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed (%08X)", true, instance.first->GetReference(), instance.first->GetBase(), instance.first->GetNetworkID(), typeid(*(instance.first)).name(), instance.first);

#endif

		Reference* reference = (Reference*) instance.first->StartSession();

		if (reference)
		{
			reference->Finalize();
			delete reference; // this throws
		}
	}

	instances.clear();

	cs.EndSession();

	// Cleanup classes

	game = 0x00;

	Object::param_Axis.first.clear();

	Container::Items = NULL;

	Actor::Actors = NULL;
	Actor::Creatures = NULL;
	Actor::param_ActorValues.first.clear();

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

	if (debug != NULL)
		debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed", true, _reference->GetReference(), _reference->GetBase(), _reference->GetNetworkID(), typeid(*_reference).name());

#endif

	cs.StartSession();

	ReferenceList::iterator it;
	it = instances.find(_reference);

	if (it != instances.end())
		instances.erase(it);

	cs.EndSession();

	_reference->Finalize();
	delete _reference; // this throws
	reference.reference = NULL;

	return id;
}
