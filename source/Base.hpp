#ifndef BASE_H
#define BASE_H

#include "vaultmp.hpp"
#include "CriticalSection.hpp"
#include "RakNet.hpp"
#include "packet/PacketFactory.hpp"

/**
 * \brief The base class for all types
 */

template<typename T>
class FactoryWrapper;

class Base : private CriticalSection, public RakNet::NetworkIDObject
{
		friend class GameFactory;

		template<typename T>
		friend class FactoryWrapper;

	private:
		Base(const Base&) = delete;
		Base& operator=(const Base&) = delete;

		virtual void initializers() {}
		virtual void freecontents() {}

	protected:
		Base();
		Base(const pPacket& packet);
		Base(pPacket&& packet) : Base(packet) {};

	public:
		virtual ~Base() noexcept;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

PF_MAKE_E(ID_BASE_NEW, pGeneratorReference)
template<>
inline const typename pTypesMap<pTypes::ID_BASE_NEW>::type* PacketFactory::Cast_<pTypes::ID_BASE_NEW>::Cast(const pPacket* packet) {
	return static_cast<const typename pTypesMap<pTypes::ID_BASE_NEW>::type*>(packet);
}

#endif
