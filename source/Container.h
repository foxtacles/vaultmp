#ifndef CONTAINER_H
#define CONTAINER_H

#define TYPECLASS
#include "GameFactory.h"

#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <cstdlib>

struct Diff
{
	signed int count;
	double condition;
	signed int equipped;
	bool silent;
	bool stick;

	Diff() : count(0), condition(0.00), equipped(0), silent(false), stick(false)
	{

	}
};

typedef std::pair<std::list<RakNet::NetworkID>, std::list<RakNet::NetworkID>> ContainerDiff;
typedef std::pair<std::list<RakNet::NetworkID>, std::vector<pPacket>> ContainerDiffNet;
typedef std::list<std::pair<unsigned int, Diff>> GameDiff;

#include "vaultmp.h"
#include "Data.h"
#include "Item.h"
#include "Object.h"
#include "PacketFactory.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

class Container : public Object
{
		friend class GameFactory;
		friend class Item;

	private:
		typedef std::pair<RakNet::NetworkID, std::unordered_map<RakNet::NetworkID, std::list<RakNet::NetworkID>>> StripCopy;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		static bool Item_sort(RakNet::NetworkID id, RakNet::NetworkID id2);
		static bool Diff_sort(const std::pair<unsigned int, Diff>& diff, const std::pair<unsigned int, Diff>& diff2);

		std::list<RakNet::NetworkID> container;
		Value<bool> flag_Lock;

		StripCopy Strip() const;

		void initialize();

		Container(const Container&) = delete;
		Container& operator=(const Container&) = delete;

	protected:
		Container(unsigned int refID, unsigned int baseID);
		Container(const pDefault* packet);
		Container(pPacket&& packet);

	public:
		virtual ~Container();

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

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

		static ContainerDiff ToContainerDiff(const ContainerDiffNet& diff);
		static ContainerDiffNet ToNetDiff(const ContainerDiff& diff);
		static void FreeDiff(ContainerDiff& diff);

		Lockable* getLock();
		bool IsEmpty() const;
		void PrintContainer() const;
		unsigned int GetItemCount(unsigned int baseID) const;
		const std::list<RakNet::NetworkID>& GetItemList() const;

#ifdef VAULTSERVER
		std::list<RakNet::NetworkID> GetItemTypes(const std::string& type) const;
#endif

		void FlushContainer();
		RakNet::NetworkID Copy() const;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#endif
