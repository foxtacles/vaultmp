#ifndef BASE_H
#define BASE_H

#include "vaultmp.h"
#include "CriticalSection.h"
#include "RakNet.h"
#include "packet/PacketFactory.h"

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

	protected:
		Base();
		Base(const pDefault* packet);
		Base(pPacket&& packet) : Base(packet.get()) {};

	public:
		virtual ~Base() noexcept;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;

		virtual void virtual_initializers() {}
};

template<> struct pTypesMap<pTypes::ID_BASE_NEW> { typedef pGeneratorReference<pTypes::ID_BASE_NEW> type; };
template<>
inline const typename pTypesMap<pTypes::ID_BASE_NEW>::type* PacketFactory::Cast_<pTypes::ID_BASE_NEW>::Cast(const pDefault* packet) {
	return static_cast<const typename pTypesMap<pTypes::ID_BASE_NEW>::type*>(packet);
}

#endif
