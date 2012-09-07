#ifndef NPC_H
#define NPC_H

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Record.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Represents a race
 */

class NPC
{
	private:
		static unordered_map<unsigned int, const NPC*> npcs;

		unsigned int baseID;
		bool essential;
		bool female;
		unsigned int race;
		unsigned int deathitem;

		NPC(const NPC&) = delete;
		NPC& operator=(const NPC& p) = delete;

	public:
		static const NPC& Lookup(unsigned int baseID);
		static const NPC& GetNPCNotIn(const unordered_set<unsigned int>& _set, const function<bool(const NPC&)>& pred);

		unsigned int GetBase() const;
		bool IsEssential() const;
		bool IsFemale() const;
		unsigned int GetRace() const;
		unsigned int GetDeathItem() const;

		NPC(const string& table, sqlite3_stmt* stmt);
		// must never be called. only defined because vector requires it
		NPC(NPC&&) { terminate(); }
		NPC& operator=(NPC&&) = delete;
		~NPC();
};

#endif
