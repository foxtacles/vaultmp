#include "NPC.h"

unordered_map<unsigned int, const NPC*> NPC::npcs;

NPC::NPC(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 6)
		throw VaultException("Malformed input database (NPCs): %s", table.c_str());

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 5));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	essential = static_cast<bool>(sqlite3_column_int(stmt, 1));
	female = static_cast<bool>(sqlite3_column_int(stmt, 2));
	race = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));
	deathitem = static_cast<unsigned int>(sqlite3_column_int(stmt, 4));

	if (race & 0xFF000000)
	{
		race &= 0x00FFFFFF;
		race |= dlc;
	}

	if (deathitem & 0xFF000000)
	{
		deathitem &= 0x00FFFFFF;
		deathitem |= dlc;
	}

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
		npcs.erase(baseID);

	npcs.emplace(baseID, this);
}

NPC::~NPC()
{
	npcs.erase(baseID);
}

const NPC& NPC::Lookup(unsigned int baseID)
{
	auto it = npcs.find(baseID);

	if (it != npcs.end())
		return *it->second;

	throw VaultException("No NPC with baseID %08X found", baseID);
}

const NPC& NPC::GetNPCNotIn(const unordered_set<unsigned int>& _set, const function<bool(const NPC&)>& pred)
{
	unordered_map<unsigned int, const NPC*>::iterator it = find_if(npcs.begin(), npcs.end(), [&](const pair<const unsigned int, const NPC*>& npcs) { return !_set.count(npcs.first) && pred(*npcs.second); });

	if (it != npcs.end())
		return *it->second;

	throw VaultException("No NPC found which is not in the given set");
}

unsigned int NPC::GetBase() const
{
	return baseID;
}

bool NPC::IsEssential() const
{
	return essential;
}

bool NPC::IsFemale() const
{
	return female;
}

unsigned int NPC::GetRace() const
{
	return race;
}

unsigned int NPC::GetDeathItem() const
{
	return deathitem;
}
