#ifndef ITEMLIST_H
#define ITEMLIST_H

#include "vaultmp.hpp"
#include "RakNet.hpp"
#include "Base.hpp"
#include "ReferenceTypes.hpp"
#include "GameFactory.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <vector>
#include <tuple>

class ItemList : public virtual Base
{
		friend class GameFactory;

	private:
		static constexpr float CONDITION_EPS = 0.001f;

		typedef std::vector<RakNet::NetworkID> Impl;

#ifdef VAULTMP_DEBUG
		static DebugInput<ItemList> debug;
#endif

		RakNet::NetworkID FindStackableItem(unsigned int baseID, float condition) const;

		Impl container;

		void initialize();

		ItemList(const ItemList&) = delete;
		ItemList& operator=(const ItemList&) = delete;

		virtual void freecontents() { container.clear(); }

	protected:
		ItemList();
		ItemList(const pPacket& packet);
		ItemList(pPacket&& packet) : ItemList(packet) {};

	public:
		typedef std::pair<bool, RakNet::NetworkID> AddOp;
		typedef std::tuple<unsigned int, Impl, RakNet::NetworkID> RemoveOp;

		virtual ~ItemList() noexcept;

		RakNet::NetworkID AddItem(RakNet::NetworkID id);
		AddOp AddItem(unsigned int baseID, unsigned int count, float condition, bool silent);
		void RemoveItem(RakNet::NetworkID id);
		RemoveOp RemoveItem(unsigned int baseID, unsigned int count, bool silent);
		RakNet::NetworkID EquipItem(unsigned int baseID, bool silent, bool stick) const;
		RakNet::NetworkID UnequipItem(unsigned int baseID, bool silent, bool stick) const;

		RakNet::NetworkID IsEquipped(unsigned int baseID) const;
		bool IsEmpty() const { return container.empty(); }
		unsigned int GetItemCount(unsigned int baseID = 0) const;
		const Impl& GetItemList() const { return container; }

#ifdef VAULTSERVER
		Impl GetItemTypes(const std::string& type) const;
#endif

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER(ItemList, Base, ID_ITEMLIST, ALL_ITEMLISTS)

PF_MAKE(ID_ITEMLIST_NEW, pGeneratorReferenceExtend, std::vector<pPacket>)
template<>
inline const typename pTypesMap<pTypes::ID_ITEMLIST_NEW>::type* PacketFactory::Cast_<pTypes::ID_ITEMLIST_NEW>::Cast(const pPacket* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_ITEMLIST_NEW ||
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_ITEMLIST_NEW>::type*>(packet) : nullptr;
}

#endif
