#ifndef TERMINALDB_H
#define TERMINALDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a terminal
 */

namespace DB
{
	class Terminal
	{
		private:
			static std::unordered_map<unsigned int, Terminal*> terminals;

			unsigned int baseID;
			unsigned int lock;
			unsigned int note;

			Terminal(const Terminal&) = delete;
			Terminal& operator=(const Terminal&) = delete;

		public:
			static Expected<Terminal*> Lookup(unsigned int baseID);

			unsigned int GetBase() const;
			unsigned int GetLock() const;
			unsigned int GetNote() const;

			Terminal(const std::string& table, sqlite3_stmt* stmt);
			~Terminal() = default;
			// must never be called. only defined because vector requires it
			Terminal(Terminal&&) { std::terminate(); }
			Terminal& operator=(Terminal&&) = delete;
	};
}

#endif
