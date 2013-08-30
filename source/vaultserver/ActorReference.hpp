#ifndef ACTORREFERENCEDB_H
#define ACTORREFERENCEDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <string>
#include <tuple>
#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a game actor reference
 */

namespace DB
{
	class ActorReference
	{
		private:
			static std::unordered_map<unsigned int, ActorReference*> refs;

			std::string editor;
			unsigned int refID;
			unsigned int baseID;
			unsigned int cell;
			std::tuple<float, float, float> pos, angle;
			unsigned int flags;

			ActorReference(const ActorReference&) = delete;
			ActorReference& operator=(const ActorReference&) = delete;

		public:
			static const std::unordered_map<unsigned int, ActorReference*>& Get() { return refs; }
			static Expected<ActorReference*> Lookup(unsigned int refID);

			const std::string& GetEditor() const;
			unsigned int GetReference() const;
			unsigned int GetBase() const;
			unsigned int GetCell() const;
			const std::tuple<float, float, float>& GetPos() const;
			const std::tuple<float, float, float>& GetAngle() const;
			unsigned int GetFlags() const;

			ActorReference(const std::string& table, sqlite3_stmt* stmt);
			~ActorReference() = default;
			// must never be called. only defined because vector requires it
			ActorReference(ActorReference&&) { std::terminate(); }
			ActorReference& operator=(ActorReference&&) = delete;
	};
}

#endif
