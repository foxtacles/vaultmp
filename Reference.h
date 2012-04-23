#ifndef REFERENCE_H
#define REFERENCE_H

#include "Data.h"
#include "Utils.h"
#include "Value.h"
#include "Network.h"
#include "CriticalSection.h"
#include "VaultFunctor.h"
#include "RakNet/NetworkIDObject.h"

using namespace std;
using namespace RakNet;
using namespace Data;

/**
 * \brief The base class for all in-game types
 *
 * Data specific to References are a reference ID, a base ID and a NetworkID
 */

class Reference : public CriticalSection, public NetworkIDObject
{
		friend class GameFactory;

	private:
		Value<unsigned int> refID;
		Value<unsigned int> baseID;

		static IndexLookup Mods;

		Reference(const Reference&);
		Reference& operator=(const Reference&);

	protected:
		static unsigned int ResolveIndex(unsigned int baseID);

		Reference(unsigned int refID, unsigned int baseID);
		virtual ~Reference();

	public:

		/**
		 * \brief Retrieves the Reference's reference ID
		 */
		unsigned int GetReference() const;
		/**
		 * \brief Retrieves the Reference's base ID
		 */
		unsigned int GetBase() const;
		/**
		 * \brief Determines if the reference ID is persistent
		 */
		bool IsPersistent() const;

		/**
		 * \brief Sets the Reference's reference ID
		 */
		Lockable* SetReference(unsigned int refID);
		/**
		 * \brief Sets the Reference's base ID
		 */
		Lockable* SetBase(unsigned int baseID);

		/**
		 * \brief Returns a constant Parameter used to pass the reference ID of this Reference to the Interface
		 */
		const Parameter GetReferenceParam() const;
		/**
		 * \brief Returns a constant Parameter used to pass the base ID of this Reference to the Interface
		 */
		const Parameter GetBaseParam() const;

		/**
		 * \brief For network transfer
		 */
		virtual pDefault* toPacket() = 0;
};

#endif
