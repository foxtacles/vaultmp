#ifndef ACREFERENCE_H
#define ACREFERENCE_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <string>
#include <tuple>
#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a game creature reference
 */

namespace DB
{
	class AcReference
	{
		private:
			static std::unordered_map<unsigned int, AcReference*> refs;

			std::string editor;
			unsigned int refID;
			unsigned int baseID;
			unsigned int cell;
			std::tuple<float, float, float> pos, angle;
			unsigned int flags;

			AcReference(const AcReference&) = delete;
			AcReference& operator=(const AcReference&) = delete;

		public:
			static const std::unordered_map<unsigned int, AcReference*>& Get() { return refs; }
			static Expected<AcReference*> Lookup(unsigned int refID);

			const std::string& GetEditor() const;
			unsigned int GetReference() const;
			unsigned int GetBase() const;
			unsigned int GetCell() const;
			const std::tuple<float, float, float>& GetPos() const;
			const std::tuple<float, float, float>& GetAngle() const;
			unsigned int GetFlags() const;

			AcReference(const std::string& table, sqlite3_stmt* stmt);
			~AcReference() = default;
			// must never be called. only defined because vector requires it
			AcReference(AcReference&&) { std::terminate(); }
			AcReference& operator=(AcReference&&) = delete;
	};
}

#endif
