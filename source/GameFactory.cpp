#include "GameFactory.h"

#include "Actor.h"
#include "Window.h"

using namespace std;
using namespace RakNet;

CriticalSection GameFactory::cs;
GameFactory::ReferenceList GameFactory::instances;
GameFactory::ReferenceIndex GameFactory::index;
GameFactory::ReferenceCount GameFactory::typecount;
GameFactory::ReferenceDeleted GameFactory::delrefs;

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

bool GameFactory::IsDeleted(NetworkID id) noexcept
{
	bool result;

	cs.StartSession();

	result = delrefs.find(id) != delrefs.end();

	cs.EndSession();

	return result;
}

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

void GameFactory::DestroyAllInstances() noexcept
{
	cs.StartSession();

	ReferenceList copy = instances;

	instances.clear();
	index.clear();
	typecount.clear();
	delrefs.clear();

	cs.EndSession();

	for (const auto& instance : copy)
	{
		Reference* reference = static_cast<Reference*>(instance.first->StartSession());

		if (instance.second & ALL_CONTAINERS)
			static_cast<Container*>(instance.first.get())->IL.container.clear();

#ifdef VAULTMP_DEBUG
		debug.print("Reference ", hex, instance.first->GetReference(), " with base ", instance.first->GetBase(), " and NetworkID ", dec, instance.first->GetNetworkID(), " (type: ", typeid(*(instance.first)).name(), ") to be destructed (", instance.first.get(), ")");
#endif

		reference->Finalize();
	}

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
