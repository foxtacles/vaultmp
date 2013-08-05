#ifndef DATABASE_H
#define DATABASE_H

#include "vaultserver.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include <string>
#include <vector>

/**
 * \brief Used to access vaultmp SQLite3 databases
 */

template<typename T>
class Database
{
		friend class GameFactory;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Database<T>> debug;
#endif

		std::vector<T> data;

		Database() = default;
		~Database() = default;

		unsigned int initialize(const std::string& file, const std::vector<std::string>& tables);
};

#endif
