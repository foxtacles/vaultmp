#ifndef NPC_H
#define NPC_H

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Record.h"
#include "BaseContainer.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Represents a NPC
 */

class NPC
{
	private:
		static unordered_map<unsigned int, const NPC*> npcs;

		unsigned int baseID;
		bool essential;
		bool female;
		unsigned int race;
		unsigned int template_;
		unsigned short flags;
		unsigned int deathitem;

		mutable signed int new_female;
		mutable unsigned int new_race;

		NPC(const NPC&) = delete;
		NPC& operator=(const NPC& p) = delete;

	public:
		static const NPC& Lookup(unsigned int baseID);
		static const NPC& GetNPCNotIn(const unordered_set<unsigned int>& _set, const function<bool(const NPC&)>& pred);

		unsigned int GetBase() const;
		bool IsEssential() const;
		bool IsFemale() const;
		bool IsOriginalFemale() const;
		unsigned int GetRace() const;
		unsigned int GetOriginalRace() const;
		unsigned int GetTemplate() const;
		unsigned short GetFlags() const;
		unsigned int GetDeathItem() const;
		const vector<const BaseContainer*>& GetBaseContainer() const;

		void SetRace(unsigned int race) const;
		void SetFemale(bool female) const;

		NPC(const string& table, sqlite3_stmt* stmt);
		// must never be called. only defined because vector requires it
		NPC(NPC&&) { terminate(); }
		NPC& operator=(NPC&&) = delete;
		~NPC();
};

#endif
