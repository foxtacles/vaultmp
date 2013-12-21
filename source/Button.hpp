#ifndef BUTTONGUI_H
#define BUTTONGUI_H

#include "vaultmp.hpp"
#include "Window.hpp"

/**
 * \brief Represents a GUI static text
 */

class Button : public Window
{
		friend class GameFactory;

	private:
		void initialize();

		Button(const Button&) = delete;
		Button& operator=(const Button&) = delete;

	protected:
		Button();

		template<typename... Args>
		Button(Args&&... args) : Window(std::forward<Args>(args)...) { initialize(); }

		Button(pPacket& packet) : Button(const_cast<const pPacket&>(packet)) {}
		Button(const pPacket& packet);
		Button(pPacket&& packet) : Button(packet) {};

	public:
		static constexpr const char* CLOSE_BUTTON = "closeBTN";

		virtual ~Button() noexcept;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(Button, Window, ID_BUTTON)

PF_MAKE_E(ID_BUTTON_NEW, pGeneratorReferenceExtend)

#endif
