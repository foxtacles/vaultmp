#ifndef LISTITEMGUI_H
#define LISTITEMGUI_H

#include "vaultmp.hpp"
#include "Base.hpp"
#include "GameFactory.hpp"
#include "ReferenceTypes.hpp"

#include <string>

/**
 * \brief Represents a GUI list item
 */

class ListItem : public Base
{
		friend class GameFactory;

	private:
		static constexpr bool DEFAULT_SELECTED = false;

		RakNet::NetworkID list;
		std::string text;
		bool selected;

		void initialize();

		ListItem(const ListItem&) = delete;
		ListItem& operator=(const ListItem&) = delete;

	protected:
		ListItem();
		ListItem(const pPacket& packet);
		ListItem(pPacket&& packet) : ListItem(packet) {};

	public:
		virtual ~ListItem() noexcept;

		void SetItemContainer(RakNet::NetworkID list) { this->list = list; }
		void SetText(const std::string& text) { this->text = text; }
		void SetSelected(bool selected) { this->selected = selected; }

		RakNet::NetworkID GetItemContainer() const { return list; }
		const std::string& GetText() const { return text; }
		bool GetSelected() const { return selected; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(ListItem, Base, ID_LISTITEM)

PF_MAKE(ID_LISTITEM_NEW, pGeneratorReferenceExtend, RakNet::NetworkID, std::string, bool)
PF_MAKE_E(ID_LISTITEM_REMOVE, pGeneratorReference)
PF_MAKE(ID_UPDATE_WLTEXT, pGeneratorReference, std::string)
PF_MAKE(ID_UPDATE_WLSELECTED, pGeneratorReference, bool)

#endif
