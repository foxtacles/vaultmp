#ifndef RADIOBUTTONGUI_H
#define RADIOBUTTONGUI_H

#include "vaultmp.hpp"
#include "Window.hpp"

/**
 * \brief Represents a GUI radio button
 */

class RadioButton : public Window
{
		friend class GameFactory;

	private:
		static constexpr bool DEFAULT_SELECTED = false;
		static constexpr unsigned int DEFAULT_GROUP = 0;

		bool selected;
		unsigned int group;

		void initialize();

		RadioButton(const RadioButton&);
		RadioButton& operator=(const RadioButton&);

	protected:
		RadioButton();
		RadioButton(const pDefault* packet);
		RadioButton(pPacket&& packet) : RadioButton(packet.get()) {};

	public:
		virtual ~RadioButton() noexcept;

		void SetSelected(bool selected) { this->selected = selected; }
		void SetGroup(unsigned int group) { this->group = group; }

		bool GetSelected() { return selected; }
		unsigned int GetGroup() { return group; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(RadioButton, Window, ID_RADIOBUTTON)

template<> struct pTypesMap<pTypes::ID_RADIOBUTTON_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_RADIOBUTTON_NEW, bool, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WRSELECTED> { typedef pGeneratorReference<pTypes::ID_UPDATE_WRSELECTED, RakNet::NetworkID, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WGROUP> { typedef pGeneratorReference<pTypes::ID_UPDATE_WGROUP, unsigned int> type; };

#endif
