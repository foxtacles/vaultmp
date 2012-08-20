#ifndef REFERENCE_H
#define REFERENCE_H

#include "Data.h"
#include "Utils.h"
#include "Value.h"
#include "Network.h"
#include "CriticalSection.h"
#include "VaultFunctor.h"
#include "RakNet/NetworkIDObject.h"
#include "Interface.h"
#include "PacketFactory.h"

using namespace std;
using namespace RakNet;

/**
 * \brief The base class for all in-game types
 *
 * Data specific to References are a reference ID, a base ID and a NetworkID
 */

class FactoryObject;

class Reference : private CriticalSection, public NetworkIDObject
{
		friend class GameFactory;
		friend class FactoryObject;

	private:
#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Value<unsigned int> refID;
		Value<unsigned int> baseID;
		Value<bool> changed;

		Reference(const Reference&) = delete;
		Reference& operator=(const Reference&) = delete;

	protected:
		//static unsigned int ResolveIndex(unsigned int baseID);

		template <typename T>
		Lockable* SetObjectValue(Value<T>& dest, T value);

		Reference(unsigned int refID, unsigned int baseID);
		virtual ~Reference();

	public:

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

		/**
		 * \brief Retrieves the Reference's reference ID
		 */
		unsigned int GetReference() const;
		/**
		 * \brief Retrieves the Reference's base ID
		 */
		unsigned int GetBase() const;
		/**
		 * \brief Retrieves the Reference's changed state
		 */
		bool GetChanged() const;
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
#ifdef VAULTSERVER
		virtual Lockable* SetBase(unsigned int baseID);
#else
		Lockable* SetBase(unsigned int baseID);
#endif
		/**
		 * \brief Sets the Reference's changed state
		 */
		Lockable* SetChanged(bool changed);

		/**
		 * \brief Returns a constant Parameter used to pass the reference ID of this Reference to the Interface
		 */
		RawParameter GetReferenceParam() const
		{
			return RawParameter(refID.get());
		};
		/**
		 * \brief Returns a constant Parameter used to pass the base ID of this Reference to the Interface
		 */
		RawParameter GetBaseParam() const
		{
			return RawParameter(baseID.get());
		};

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const = 0;
};

class ReferenceFunctor : public VaultFunctor
{
	private:
		unsigned int _flags;
		NetworkID id;

	protected:
		ReferenceFunctor(unsigned int flags, NetworkID id) : VaultFunctor(), _flags(flags), id(id) {}
		virtual ~ReferenceFunctor() {}

		virtual bool filter(FactoryObject& reference) = 0;

		unsigned int flags() { return _flags; }
		NetworkID get() { return id; }
};

#endif
