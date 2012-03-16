#ifndef ITEM_H
#define ITEM_H

#define TYPECLASS
#include "GameFactory.h"

#include "vaultmp.h"
#include "Data.h"
#include "Value.h"
#include "Object.h"
#include "Container.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace Data;
using namespace std;

class Container;

class Item : public Object
{
		friend class GameFactory;
		friend class Container;

	private:
#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Database::const_iterator data;

		Value<unsigned int> item_Count;
		Value<double> item_Condition;
		Value<bool> state_Equipped;

		Item( const Item& );
		Item& operator=( const Item& );

	protected:
		Item( unsigned int refID, unsigned int baseID );
		virtual ~Item();

	public:
#ifdef VAULTMP_DEBUG
		static void SetDebugHandler( Debug* debug );
#endif

		unsigned int GetItemCount() const;
		double GetItemCondition() const;
		bool GetItemEquipped() const;

		bool SetItemCount( unsigned int count );
		bool SetItemCondition( double condition );
		bool SetItemEquipped( bool state );

		NetworkID Copy() const;

		/**
		 * \brief For network transfer
		 */
        virtual pDefault* toPacket();
};

#endif
