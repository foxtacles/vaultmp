#ifndef WEAPONDB_H
#define WEAPONDB_H

#include <unordered_map>

#include "vaultmp.hpp"
#include "vaultserver.hpp"
#include "Database.hpp"
#include "Expected.hpp"

#include "vaultserver.hpp"

class sqlite3_stmt;

/**
 * \brief Represents a game weapon
 */

namespace DB
{
	class Weapon
	{
		private:
			static std::unordered_map<unsigned int, Weapon*> weapons;

			unsigned int baseID;
			float damage;
			float reload;
			float rate;
			bool automatic;
			unsigned int ammo;

			Weapon(const Weapon&) = delete;
			Weapon& operator=(const Weapon&) = delete;

		public:
			static Expected<Weapon*> Lookup(unsigned int baseID);

			unsigned int GetBase() const;
			float GetDamage() const;
			float GetReloadTime() const;
			float GetFireRate() const;
			bool IsAutomatic() const;
			unsigned int GetAmmo() const;

			Weapon(const std::string& table, sqlite3_stmt* stmt);
			~Weapon() = default;
			// must never be called. only defined because vector requires it
			Weapon(Weapon&&) { std::terminate(); }
			Weapon& operator=(Weapon&&) = delete;
	};
}

#endif
