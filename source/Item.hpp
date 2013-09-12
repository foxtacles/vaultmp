#ifndef ITEM_H
#define ITEM_H

#include "vaultmp.hpp"
#include "Object.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

class Container;

class Item : public Object
{
		friend class GameFactory;
		friend class Container;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Item> debug;
#endif

		Value<RakNet::NetworkID> item_Container;
		Value<unsigned int> item_Count;
		Value<float> item_Condition;
		Value<bool> state_Equipped;
		Value<bool> flag_Silent;
		Value<bool> flag_Stick;

		void initialize();

		Item(const Item&) = delete;
		Item& operator=(const Item&) = delete;

	protected:
		Item(unsigned int refID, unsigned int baseID);
		Item(unsigned int baseID) : Item(0x00000000, baseID) {}
		Item(const pPacket& packet);

	public:
		virtual ~Item() noexcept;

		RakNet::NetworkID GetItemContainer() const;
		unsigned int GetItemCount() const;
		float GetItemCondition() const;
		bool GetItemEquipped() const;
		bool GetItemSilent() const;
		bool GetItemStick() const;

		Lockable* SetItemContainer(RakNet::NetworkID id);
		Lockable* SetItemCount(unsigned int count);
		Lockable* SetItemCondition(float condition);
		Lockable* SetItemEquipped(bool state);
		Lockable* SetItemSilent(bool silent);
		Lockable* SetItemStick(bool stick);

#ifdef VAULTSERVER
		/**
		 * \brief Sets the Item's base ID
		 */
		virtual Lockable* SetBase(unsigned int baseID);
#endif

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(Item, Object, ID_ITEM)

template<> struct pTypesMap<pTypes::ID_ITEM_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_ITEM_NEW, RakNet::NetworkID, unsigned int, float, bool, bool, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_COUNT> { typedef pGeneratorReference<pTypes::ID_UPDATE_COUNT, unsigned int, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_CONDITION> { typedef pGeneratorReference<pTypes::ID_UPDATE_CONDITION, float, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_EQUIPPED> { typedef pGeneratorReference<pTypes::ID_UPDATE_EQUIPPED, bool, bool, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_VALUE> { typedef pGeneratorReference<pTypes::ID_UPDATE_VALUE, bool, unsigned char, float> type; };

#endif
