#ifndef BUTTONGUI_H
#define BUTTONGUI_H

#include "vaultmp.h"
#include "Window.h"

/**
 * \brief Represents a GUI static text
 */

class Button : public Window
{
		friend class GameFactory;

	private:
		void initialize();

		Button(const Button&);
		Button& operator=(const Button&);

	protected:
		Button();
		Button(const pDefault* packet);
		Button(pPacket&& packet) : Button(packet.get()) {};

	public:
		static constexpr const char* CLOSE_BUTTON = "closeBTN";

		virtual ~Button() noexcept;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER(Button, Window, ID_BUTTON)
template<> struct rTypes<Button> { enum { value = ID_BUTTON }; };

template<> struct pTypesMap<pTypes::ID_BUTTON_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_BUTTON_NEW> type; };

#endif
