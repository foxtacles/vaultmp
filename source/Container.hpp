#ifndef CONTAINER_H
#define CONTAINER_H

#include "vaultmp.hpp"
#include "Object.hpp"
#include "ItemList.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

class Container : public Object, public ItemList
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
		Container(unsigned int baseID) : Container(0x00000000, baseID) {}
		Container(const pPacket& packet);
		Container(pPacket&& packet) : Container(packet) {};

	public:
		virtual ~Container() noexcept;

#ifndef VAULTSERVER
		/**
		 * \brief Creates a Parameter containing a VaultFunctor initialized with the given flags
		 *
		 * Used to pass Container references matching the provided flags to the Interface
		 * Can also be used to pass data of a given Container to the Interface
		 */
		static FuncParameter CreateFunctor(unsigned int flags, RakNet::NetworkID id = 0);
#endif

#ifdef VAULTSERVER
		/**
		 * \brief Sets the Container's base ID
		 */
		virtual Lockable* SetBase(unsigned int baseID);
#endif

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

GF_TYPE_WRAPPER(Container, Object, ID_CONTAINER, ALL_CONTAINERS)

PF_MAKE(ID_CONTAINER_NEW, pGeneratorReferenceExtend, pPacket)
template<>
inline const typename pTypesMap<pTypes::ID_CONTAINER_NEW>::type* PacketFactory::Cast_<pTypes::ID_CONTAINER_NEW>::Cast(const pPacket* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_CONTAINER_NEW>::type*>(packet) : nullptr;
}

#endif
