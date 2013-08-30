#ifndef ITEMDB_H
#define ITEMDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents an item
 */

namespace DB
{
	class Item
	{
		private:
			static std::unordered_map<unsigned int, Item*> items;

			unsigned int baseID;
			unsigned int value;
			unsigned int health;
			float weight;
			unsigned int slot;

			Item(const Item&) = delete;
			Item& operator=(const Item&) = delete;

		public:
			static Expected<Item*> Lookup(unsigned int baseID);

			unsigned int GetBase() const;
			unsigned int GetValue() const;
			unsigned int GetHealth() const;
			float GetWeight() const;
			unsigned int GetSlot() const;

			Item(const std::string& table, sqlite3_stmt* stmt);
			~Item() = default;
			// must never be called. only defined because vector requires it
			Item(Item&&) { std::terminate(); }
			Item& operator=(Item&&) = delete;
	};
}

#endif
