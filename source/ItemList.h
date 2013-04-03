#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <list>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>

#include "vaultmp.h"
#include "RakNet.h"
#include "PacketFactory.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

class Item;

class ItemList
{
		friend class GameFactory;

	public:
		struct Diff
		{
			signed int count;
			double condition;
			signed int equipped;
			bool silent;
			bool stick;

			Diff() : count(0), condition(0.00), equipped(0), silent(false), stick(false) {}
		};

		typedef std::list<RakNet::NetworkID> ItemListImpl;
		typedef std::pair<ItemListImpl, ItemListImpl> ContainerDiff;
		typedef std::pair<ItemListImpl, std::vector<pPacket>> NetDiff;
		typedef std::list<std::pair<unsigned int, Diff>> GameDiff;

	private:
		typedef std::pair<RakNet::NetworkID, std::unordered_map<RakNet::NetworkID, ItemListImpl>> StripCopy;

#ifdef VAULTMP_DEBUG
		static DebugInput<ItemList> debug;
#endif

		static bool Item_sort(RakNet::NetworkID id, RakNet::NetworkID id2);
		static bool Diff_sort(const std::pair<unsigned int, Diff>& diff, const std::pair<unsigned int, Diff>& diff2);

		ItemListImpl container;
		RakNet::NetworkID source;
		StripCopy Strip() const;

		ItemList(const ItemList&) = delete;
		ItemList& operator=(const ItemList&) = delete;

	public:
		ItemList(RakNet::NetworkID source) : source(source) {}
		~ItemList();

		ItemList(ItemList&&) = default;
		ItemList& operator=(ItemList&&) = default;

		void AddItem(RakNet::NetworkID id);
		ContainerDiff AddItem(unsigned int baseID, unsigned int count, double condition, bool silent) const;
		void RemoveItem(RakNet::NetworkID id);
		ContainerDiff RemoveItem(unsigned int baseID, unsigned int count, bool silent) const;
		ContainerDiff RemoveAllItems() const;
		ContainerDiff EquipItem(unsigned int baseID, bool silent, bool stick) const;
		ContainerDiff UnequipItem(unsigned int baseID, bool silent, bool stick) const;

		ContainerDiff Compare(RakNet::NetworkID id) const;
		RakNet::NetworkID IsEquipped(unsigned int baseID) const;
		GameDiff ApplyDiff(ContainerDiff& diff);

		static ContainerDiff ToContainerDiff(const NetDiff& diff);
		static NetDiff ToNetDiff(const ContainerDiff& diff);
		static void FreeDiff(ContainerDiff& diff);

		bool IsEmpty() const;
		unsigned int GetItemCount(unsigned int baseID = 0) const;
		const ItemListImpl& GetItemList() const;

#ifdef VAULTSERVER
		ItemListImpl GetItemTypes(const std::string& type) const;
#endif

		void FlushContainer();
		ItemList Copy(RakNet::NetworkID source) const;
};

#endif
