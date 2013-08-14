#ifndef ITEMLIST_H
#define ITEMLIST_H

#include "vaultmp.h"
#include "packet/PacketFactory.h"
#include "RakNet.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include <vector>
#include <tuple>

#ifdef VAULTSERVER
class ItemList : public RakNet::NetworkIDObject
#else
class ItemList
#endif
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

		RakNet::NetworkID source;
		Impl container;

		ItemList(const ItemList&) = delete;
		ItemList& operator=(const ItemList&) = delete;

	public:
		ItemList(RakNet::NetworkID source);
		~ItemList() noexcept;

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
};

#endif
