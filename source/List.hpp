#ifndef LISTGUI_H
#define LISTGUI_H

#include "vaultmp.hpp"
#include "Window.hpp"

#include <vector>

/**
 * \brief Represents a GUI list
 */

class List : public Window
{
		friend class GameFactory;

	private:
		typedef std::vector<RakNet::NetworkID> Impl;

		Impl container;

		void initialize();

		List(const List&) = delete;
		List& operator=(const List&) = delete;

		virtual void freecontents() { container.clear(); }

	protected:
		List();
		List(const pDefault* packet);
		List(pPacket&& packet) : List(packet.get()) {};

	public:
		virtual ~List() noexcept;

		void AddItem(RakNet::NetworkID id);
		void RemoveItem(RakNet::NetworkID id);
		Impl RemoveAllItems();

		const Impl& GetItemList() const { return container; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(List, Window, ID_LIST)

template<> struct pTypesMap<pTypes::ID_LIST_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_LIST_NEW, std::vector<pPacket>> type; };

#endif
