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

		Checkbox(const Checkbox&);
		Checkbox& operator=(const Checkbox&);

	protected:
		Checkbox();
		Checkbox(const pDefault* packet);
		Checkbox(pPacket&& packet) : Checkbox(packet.get()) {};

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

template<> struct pTypesMap<pTypes::ID_CHECKBOX_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_CHECKBOX_NEW, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WSELECTED> { typedef pGeneratorReference<pTypes::ID_UPDATE_WSELECTED, bool> type; };

#endif
