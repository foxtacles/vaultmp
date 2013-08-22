#ifndef ITEMLIST_H
#define ITEMLIST_H

#include "vaultmp.hpp"
#include "RakNet.hpp"
#include "Base.hpp"
#include "ReferenceTypes.hpp"
#include "GameFactory.hpp"
#include "packet/PacketFactory.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <vector>
#include <tuple>

class ItemList : public virtual Base
{
		friend class GameFactory;

	private:
		static constexpr double CONDITION_EPS = 0.001;

		typedef std::vector<RakNet::NetworkID> Impl;
		typedef std::pair<bool, RakNet::NetworkID> AddOp;
		typedef std::tuple<unsigned int, Impl, RakNet::NetworkID> RemoveOp;

#ifdef VAULTMP_DEBUG
		static DebugInput<ItemList> debug;
#endif

		RakNet::NetworkID FindStackableItem(unsigned int baseID, double condition) const;

		Impl container;

		void initialize();

		ItemList(const ItemList&) = delete;
		ItemList& operator=(const ItemList&) = delete;

	protected:
		ItemList();
		ItemList(const pDefault* packet);
		ItemList(pPacket&& packet) : ItemList(packet.get()) {};

	public:
		virtual ~ItemList() noexcept;

		RakNet::NetworkID AddItem(RakNet::NetworkID id);
		AddOp AddItem(unsigned int baseID, unsigned int count, double condition, bool silent);
		void RemoveItem(RakNet::NetworkID id);
		RemoveOp RemoveItem(unsigned int baseID, unsigned int count, bool silent);
		Impl RemoveAllItems();
		RakNet::NetworkID EquipItem(unsigned int baseID, bool silent, bool stick) const;
		RakNet::NetworkID UnequipItem(unsigned int baseID, bool silent, bool stick) const;

		RakNet::NetworkID IsEquipped(unsigned int baseID) const;
		bool IsEmpty() const;
		unsigned int GetItemCount(unsigned int baseID = 0) const;
		const Impl& GetItemList() const;

#ifdef VAULTSERVER
		Impl GetItemTypes(const std::string& type) const;
#endif

		void FlushContainer();
		void Copy(ItemList& IL) const;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER(ItemList, Base, ID_ITEMLIST, ALL_ITEMLISTS)

template<> struct pTypesMap<pTypes::ID_ITEMLIST_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_ITEMLIST_NEW, std::vector<pPacket>> type; };
template<>
inline const typename pTypesMap<pTypes::ID_ITEMLIST_NEW>::type* PacketFactory::Cast_<pTypes::ID_ITEMLIST_NEW>::Cast(const pDefault* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_ITEMLIST_NEW ||
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_ITEMLIST_NEW>::type*>(packet) : nullptr;
}

#endif
