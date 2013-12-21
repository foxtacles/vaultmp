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

		template<typename... Args>
		Checkbox(Args&&... args) : Window(std::forward<Args>(args)...), selected(DEFAULT_SELECTED) { initialize(); }

		Checkbox(pPacket& packet) : Checkbox(const_cast<const pPacket&>(packet)) {}
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

PF_MAKE(ID_CHECKBOX_NEW, pGeneratorReferenceExtend, bool)
PF_MAKE(ID_UPDATE_WSELECTED, pGeneratorReference, bool)

#endif
