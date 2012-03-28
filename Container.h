#ifndef CONTAINER_H
#define CONTAINER_H

#define TYPECLASS
#include "GameFactory.h"

#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include "vaultmp.h"
#include "Data.h"
#include "Item.h"
#include "Object.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace Data;
using namespace std;

class Item;

struct Diff
{
	signed int count;
	double condition;
	signed int equipped;

	Diff()
	{
		count = 0;
		condition = 0.00;
		equipped = 0;
	}
};

typedef pair<list<NetworkID>, list<NetworkID> > ContainerDiff;
typedef list<pair<unsigned int, Diff> > GameDiff;
typedef pair<NetworkID, map<NetworkID, list<NetworkID> > > StripCopy;

class Container : public Object
{
		friend class GameFactory;
		friend class Item;

	private:
		static Database Fallout3Items;
		static Database FalloutNVItems;

		static Database* Items;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		static bool Item_sort( NetworkID id, NetworkID id2 );
		static bool Diff_sort( pair<unsigned int, Diff> diff, pair<unsigned int, Diff> diff2 );

		list<NetworkID> container;

		StripCopy Strip() const;

        void initialize();

		Container( const Container& );
		Container& operator=( const Container& );

	protected:
		Container( unsigned int refID, unsigned int baseID );
		Container(const pDefault* packet);
		Container(pDefault* packet);
		virtual ~Container();

	public:

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler( Debug* debug );
#endif

		void AddItem( NetworkID id );
		void RemoveItem( NetworkID id );
		ContainerDiff Compare( NetworkID id ) const;
		GameDiff ApplyDiff( ContainerDiff& diff );
		static void FreeDiff( ContainerDiff& diff );

		bool IsEmpty() const;
		void PrintContainer() const;
		void FlushContainer();
		const list<NetworkID>& GetItemList() const;

		NetworkID Copy() const;

		/**
		 * \brief For network transfer
		 */
        virtual pDefault* toPacket();
};

#endif
