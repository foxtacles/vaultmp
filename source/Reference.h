#ifndef REFERENCE_H
#define REFERENCE_H

#include "vaultmp.h"
#include "CriticalSection.h"
#include "Value.h"
#include "RakNet.h"
#include "VaultFunctor.h"
#include "packet/PacketFactory.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include <queue>

/**
 * \brief The base class for all in-game types
 *
 * Data specific to References are a reference ID, a base ID and a NetworkID
 */

template<typename T>
class FactoryWrapper;

class Reference : private CriticalSection, public RakNet::NetworkIDObject
{
		friend class GameFactory;

		template<typename T>
		friend class FactoryWrapper;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Reference> debug;
#endif

#ifndef VAULTSERVER
		std::queue<std::function<void()>> tasks;
#endif

		Reference(const Reference&) = delete;
		Reference& operator=(const Reference&) = delete;

	protected:
		//static unsigned int ResolveIndex(unsigned int baseID);

		template<typename T> static Lockable* SetObjectValue(Value<T>& dest, const T& value);

		Reference();

	public:
		virtual ~Reference() noexcept;

#ifndef VAULTSERVER
		/**
		 * \brief Enqueues a task
		 */
		void Enqueue(const std::function<void()>& task);
		/**
		 * \brief Executes all tasks
		 */
		void Work();
		/**
		 * \brief Releases all tasks
		 */
		void Release();
#else
		virtual void virtual_initializers() {}
#endif

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const = 0;
};

template<typename T>
Lockable* Reference::SetObjectValue(Value<T>& dest, const T& value)
{
	if (dest.get() == value)
		return nullptr;

	if (!dest.set(value))
		return nullptr;

	return &dest;
}

template<> Lockable* Reference::SetObjectValue(Value<double>& dest, const double& value);

class ReferenceFunctor : public VaultFunctor
{
	private:
		unsigned int _flags;
		RakNet::NetworkID id;

	protected:
		ReferenceFunctor(unsigned int flags, RakNet::NetworkID id) : VaultFunctor(), _flags(flags), id(id) {}
		virtual ~ReferenceFunctor() {}

		virtual bool filter(FactoryWrapper<Reference>& reference) = 0;

		unsigned int flags() { return _flags; }
		RakNet::NetworkID get() { return id; }
};

#endif
