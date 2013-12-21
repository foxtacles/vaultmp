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

		static constexpr bool DEFAULT_MULTISELECT = false;

		Impl container;
		bool multiselect;

		void initialize();

		List(const List&) = delete;
		List& operator=(const List&) = delete;

		virtual void freecontents() { container.clear(); }

	protected:
		List();

		template<typename... Args>
		List(Args&&... args) : Window(std::forward<Args>(args)...), multiselect(DEFAULT_MULTISELECT) { initialize(); }

		List(pPacket& packet) : List(const_cast<const pPacket&>(packet)) {}
		List(const pPacket& packet);
		List(pPacket&& packet) : List(packet) {};

	public:
		virtual ~List() noexcept;

		void AddItem(RakNet::NetworkID id);
		void RemoveItem(RakNet::NetworkID id);
		Impl RemoveAllItems();
		void SetMultiSelect(bool multiselect) { this->multiselect = multiselect; }

		const Impl& GetItemList() const { return container; }
		bool GetMultiSelect() const { return multiselect; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(List, Window, ID_LIST)

PF_MAKE(ID_LIST_NEW, pGeneratorReferenceExtend, std::vector<pPacket>, bool)
PF_MAKE(ID_UPDATE_WLMULTI, pGeneratorReference, bool)

#endif
