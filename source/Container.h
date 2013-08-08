#ifndef CONTAINER_H
#define CONTAINER_H

#include "vaultmp.h"
#include "Object.h"
#include "ItemList.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

class Container : public Object
{
		friend class GameFactory;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Container> debug;
#endif

		void initialize();

		Container(const Container&) = delete;
		Container& operator=(const Container&) = delete;

	protected:
		Container(unsigned int refID, unsigned int baseID);
		Container(const pDefault* packet);
		Container(pPacket&& packet);

	public:
		virtual ~Container();
#ifndef VAULTSERVER
		/**
		 * \brief Creates a Parameter containing a VaultFunctor initialized with the given flags
		 *
		 * Used to pass Container references matching the provided flags to the Interface
		 * Can also be used to pass data of a given Container to the Interface
		 */
		static FuncParameter CreateFunctor(unsigned int flags, RakNet::NetworkID id = 0);
#endif

		ItemList IL;

#ifdef VAULTSERVER
		/**
		 * \brief Sets the Container's base ID
		 */
		virtual Lockable* SetBase(unsigned int baseID);
#endif

		RakNet::NetworkID Copy() const;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#ifndef VAULTSERVER
class ContainerFunctor : public ObjectFunctor
{
	public:
		ContainerFunctor(unsigned int flags, RakNet::NetworkID id) : ObjectFunctor(flags, id) {}
		virtual ~ContainerFunctor() {}

		virtual std::vector<std::string> operator()();
		virtual bool filter(FactoryWrapper<Reference>& reference);
};
#endif

#endif
