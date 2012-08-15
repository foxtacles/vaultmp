#include "Weapon.h"

unordered_map<unsigned int, const Weapon*> Weapon::weapons;

Weapon::Weapon(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 5)
		throw VaultException("Malformed input database (weapons): %s", table.c_str());

	unsigned char dlc = static_cast<unsigned char>(sqlite3_column_int(stmt, 4));
	// if DLC enabled

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	damage = sqlite3_column_double(stmt, 1);
	reload = sqlite3_column_double(stmt, 2);
	attacks = sqlite3_column_double(stmt, 3);

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= (static_cast<unsigned int>(dlc) << 24);
	}
	else
		weapons.erase(baseID);

	weapons.emplace(baseID, this);
}

Weapon::~Weapon()
{
	weapons.erase(baseID);
}

const Weapon& Weapon::Lookup(unsigned int baseID)
{
	unordered_map<unsigned int, const Weapon*>::iterator it = weapons.find(baseID);

	if (it != weapons.end())
		return *it->second;

	throw VaultException("No weapon with baseID %08X found", baseID);
}

unsigned int Weapon::GetBase() const
{
	return baseID;
}

double Weapon::GetDamage() const
{
	return damage;
}

double Weapon::GetReloadTime() const
{
	return reload;
}

double Weapon::GetAttacksSec() const
{
	return attacks;
}
