#include "NPC.hpp"
#include "Race.hpp"
#include "sqlite/sqlite3.h"

#include <algorithm>

using namespace std;
using namespace DB;

unordered_map<unsigned int, NPC*> NPC::npcs;

NPC::NPC(const string& table, sqlite3_stmt* stmt) : new_female(-1), new_race(0x00000000)
{
	if (sqlite3_column_count(stmt) != 8)
		throw VaultException("Malformed input database (NPCs): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 7));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	essential = static_cast<bool>(sqlite3_column_int(stmt, 1));
	female = static_cast<bool>(sqlite3_column_int(stmt, 2));
	race = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));
	template_ = static_cast<unsigned int>(sqlite3_column_int(stmt, 4));
	flags = static_cast<unsigned short>(sqlite3_column_int(stmt, 5));
	deathitem = static_cast<unsigned int>(sqlite3_column_int(stmt, 6));

	if (race & 0xFF000000)
	{
		race &= 0x00FFFFFF;
		race |= dlc;
	}

	if (template_ & 0xFF000000)
	{
		template_ &= 0x00FFFFFF;
		template_ |= dlc;
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

Expected<NPC*> NPC::Lookup(unsigned int baseID)
{
	auto it = npcs.find(baseID);

	if (it != npcs.end())
		return it->second;

	return VaultException("No NPC with baseID %08X found", baseID);
}

Expected<NPC*> NPC::GetNPC(const function<bool(const NPC&)>& pred)
{
	auto it = find_if(npcs.begin(), npcs.end(), [&pred](const pair<const unsigned int, const NPC*>& npcs) { return pred(*npcs.second); });

	if (it != npcs.end())
		return it->second;

	return VaultException("No NPC found fulfilling the conditions of the given function");
}

unsigned int NPC::GetBase() const
{
	return baseID;
}

bool NPC::IsEssential() const
{
	if (template_ && (flags & TplFlags::Base))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->IsEssential();
	}

	return essential;
}

bool NPC::IsFemale() const
{
	if (template_ && (flags & TplFlags::Traits))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->IsFemale();
	}

	return ((new_female != -1) ? new_female : female);
}

bool NPC::IsOriginalFemale() const
{
	if (template_ && (flags & TplFlags::Traits))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->IsOriginalFemale();
	}

	return female;
}

unsigned int NPC::GetRace() const
{
	if (template_ && (flags & TplFlags::Traits))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->GetRace();
	}

	return (new_race ? new_race : race);
}

unsigned int NPC::GetOriginalRace() const
{
	if (template_ && (flags & TplFlags::Traits))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->GetOriginalRace();
	}

	return race;
}

unsigned int NPC::GetTemplate() const
{
	return template_;
}

unsigned short NPC::GetFlags() const
{
	return flags;
}

unsigned int NPC::GetDeathItem() const
{
	if (template_ && (flags & TplFlags::Traits))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->GetDeathItem();
	}

	return deathitem;
}

const vector<BaseContainer*>& NPC::GetBaseContainer() const
{
	if (template_ && (flags & TplFlags::Inventory))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->GetBaseContainer();
	}

	return BaseContainer::Lookup(baseID);
}

void NPC::SetRace(unsigned int race)
{
	if (template_ && (flags & TplFlags::Traits))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->SetRace(race);
	}

	if (Race::Lookup(race))
		this->new_race = race;
}

void NPC::SetFemale(bool female)
{
	if (template_ && (flags & TplFlags::Traits))
	{
		auto npc = Lookup(template_);

		if (npc)
			return npc->SetFemale(female);
	}

	this->new_female = female;
}
