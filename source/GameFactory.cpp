#include "GameFactory.hpp"

using namespace std;
using namespace RakNet;

Guarded<> GameFactory::cs;
GameFactory::BaseList GameFactory::instances;
GameFactory::BaseIndex GameFactory::index;
GameFactory::BaseCount GameFactory::typecount;
GameFactory::BaseDeleted GameFactory::delrefs;

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
Database<DB::AcReference> GameFactory::dbAcReferences;
#endif

void GameFactory::Initialize()
{
#ifdef VAULTSERVER
	dbRecords.initialize(DB_FALLOUT3, {"CONT", "NPC_", "CREA", "LVLI", "ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "ENCH", "KEYM", "MISC", "NOTE", "WEAP", "CELL", "IDLE", "WTHR", "STAT", "MSTT", "RACE", "LIGH", "DOOR", "TERM", "EXPL", "PROJ", "STAT", "SOUN"});
	dbExteriors.initialize(DB_FALLOUT3, {"exteriors"});
	dbWeapons.initialize(DB_FALLOUT3, {"weapons"});
	dbRaces.initialize(DB_FALLOUT3, {"races"});
	dbNpcs.initialize(DB_FALLOUT3, {"npcs"});
	dbContainers.initialize(DB_FALLOUT3, {"npcitems", "contitems"});
	dbItems.initialize(DB_FALLOUT3, {"items"});
	dbTerminals.initialize(DB_FALLOUT3, {"terminals"});
	dbInteriors.initialize(DB_FALLOUT3, {"interiors"});
	dbAcReferences.initialize(DB_FALLOUT3, {"arefs", "crefs"});
	dbReferences.initialize(DB_FALLOUT3, {"refs_CONT", "refs_DOOR", "refs_TERM", "refs_STAT"});
#endif
}

vector<NetworkID> GameFactory::GetByType(unsigned int type) noexcept
{
	return cs.Operate([type]() {
		vector<NetworkID> result;
		result.reserve(typecount[type]);

		for (const auto& reference : instances)
			if (reference.second & type)
				result.emplace_back(reference.first->GetNetworkID());

		return result;
	});
}

unsigned int GameFactory::GetCount(unsigned int type) noexcept
{
	return cs.Operate([type]() {
		return typecount[type];
	});
}

bool GameFactory::IsDeleted(NetworkID id) noexcept
{
	return cs.Operate([id]() {
		return delrefs.find(id) != delrefs.end();
	});
}

unsigned int GameFactory::GetType(NetworkID id) noexcept
{
	return cs.Operate([id]() {
		auto it = GetShared(id);
		return it != instances.end() ? it->second : 0x00000000;
	});
}

void GameFactory::DestroyAll() noexcept
{
	BaseList copy;

	cs.Operate([&copy]() {
		copy = move(instances);
		index.clear();
		typecount.clear();
		delrefs.clear();
	});

	for (const auto& instance : copy)
	{
		Base* reference = static_cast<Base*>(instance.first->StartSession());
		reference->freecontents();

#ifdef VAULTMP_DEBUG
		debug.print("Base ", std::dec, instance.first->GetNetworkID(), " (type: ", typeid(*(instance.first)).name(), ") to be destructed (", instance.first.get(), ")");
#endif

		reference->Finalize();
	}

	Lockable::Reset();
}

bool GameFactory::Destroy(NetworkID id)
{
	return Destroy(Get<Base>(id).get());
}
