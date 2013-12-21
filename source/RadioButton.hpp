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

		RadioButton(const RadioButton&) = delete;
		RadioButton& operator=(const RadioButton&) = delete;

	protected:
		RadioButton();

		template<typename... Args>
		RadioButton(Args&&... args) : Window(std::forward<Args>(args)...), selected(DEFAULT_SELECTED), group(DEFAULT_GROUP) { initialize(); }

		RadioButton(pPacket& packet) : RadioButton(const_cast<const pPacket&>(packet)) {}
		RadioButton(const pPacket& packet);
		RadioButton(pPacket&& packet) : RadioButton(packet) {};

	public:
		virtual ~RadioButton() noexcept;

		void SetSelected(bool selected) { this->selected = selected; }
		void SetGroup(unsigned int group) { this->group = group; }

		bool GetSelected() const { return selected; }
		unsigned int GetGroup() const { return group; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(RadioButton, Window, ID_RADIOBUTTON)

PF_MAKE(ID_RADIOBUTTON_NEW, pGeneratorReferenceExtend, bool, unsigned int)
PF_MAKE(ID_UPDATE_WRSELECTED, pGeneratorReference, RakNet::NetworkID, bool)
PF_MAKE(ID_UPDATE_WGROUP, pGeneratorReference, unsigned int)

#endif
