#ifndef CHECKBOXGUI_H
#define CHECKBOXGUI_H

#include "vaultmp.hpp"
#include "Window.hpp"

/**
 * \brief Represents a GUI checkbox
 */

class Checkbox : public Window
{
		friend class GameFactory;

	private:
		static constexpr bool DEFAULT_SELECTED = false;

		bool selected;

		void initialize();

		Checkbox(const Checkbox&) = delete;
		Checkbox& operator=(const Checkbox&) = delete;

	protected:
		Checkbox();
		Checkbox(const pPacket& packet);
		Checkbox(pPacket&& packet) : Checkbox(packet) {};

	public:
		virtual ~Checkbox() noexcept;

		void SetSelected(bool selected) { this->selected = selected; }

		bool GetSelected() const { return selected; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(Checkbox, Window, ID_CHECKBOX)

PF_PACKET(ID_CHECKBOX_NEW, pGeneratorReferenceExtend, bool)
PF_PACKET(ID_UPDATE_WSELECTED, pGeneratorReference, bool)

#endif
