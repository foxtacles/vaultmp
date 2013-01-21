#ifndef ITEMDB_H
#define ITEMDB_H

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Expected.h"

#include "sqlite/sqlite3.h"

/**
 * \brief Represents an item
 */

namespace DB
{
	class Item
	{
		private:
			static std::unordered_map<unsigned int, const Item*> items;

			unsigned int baseID;
			unsigned int value;
			unsigned int health;
			double weight;
			unsigned int slot;

			Item(const Item&) = delete;
			Item& operator=(const Item&) = delete;

		public:
			static Expected<const Item*> Lookup(unsigned int baseID);

			unsigned int GetBase() const;
			unsigned int GetValue() const;
			unsigned int GetHealth() const;
			double GetWeight() const;
			unsigned int GetSlot() const;

			Item(const std::string& table, sqlite3_stmt* stmt);
			~Item() = default;
			// must never be called. only defined because vector requires it
			Item(Item&&) { std::terminate(); }
			Item& operator=(Item&&) = delete;
	};
}

#endif
