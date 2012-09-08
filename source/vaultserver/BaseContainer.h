#ifndef BASECONTAINER_H
#define BASECONTAINER_H

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Record.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Represents a base container
 */

class BaseContainer
{
	private:
		static unordered_map<unsigned int, vector<const BaseContainer*>> baseContainers;

		unsigned int baseID;
		unsigned int item;
		unsigned int count;
		double condition;

		BaseContainer(const BaseContainer&) = delete;
		BaseContainer& operator=(const BaseContainer& p) = delete;

	public:
		static const vector<const BaseContainer*>& Lookup(unsigned int baseID);

		unsigned int GetBase() const;
		unsigned int GetItem() const;
		unsigned int GetCount() const;
		double GetCondition() const;

		BaseContainer(const string& table, sqlite3_stmt* stmt);
		// must never be called. only defined because vector requires it
		BaseContainer(BaseContainer&&) { terminate(); }
		BaseContainer& operator=(BaseContainer&&) = delete;
		~BaseContainer();
};

#endif
