#ifndef BASECONTAINERDB_H
#define BASECONTAINERDB_H

#include "vaultserver.hpp"

#include <vector>
#include <string>
#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a base container
 */

namespace DB
{
	class BaseContainer
	{
		private:
			static std::unordered_map<unsigned int, std::vector<BaseContainer*>> baseContainers;

			unsigned int baseID;
			unsigned int item;
			unsigned int count;
			float condition;

			BaseContainer(const BaseContainer&) = delete;
			BaseContainer& operator=(const BaseContainer&) = delete;

		public:
			static const std::vector<BaseContainer*>& Lookup(unsigned int baseID);

			unsigned int GetBase() const;
			unsigned int GetItem() const;
			unsigned int GetCount() const;
			float GetCondition() const;

			BaseContainer(const std::string& table, sqlite3_stmt* stmt);
			~BaseContainer() = default;
			// must never be called. only defined because vector requires it
			BaseContainer(BaseContainer&&) { std::terminate(); }
			BaseContainer& operator=(BaseContainer&&) = delete;
	};
}

#endif
