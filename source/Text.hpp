#ifndef TEXTGUI_H
#define TEXTGUI_H

#include "vaultmp.hpp"
#include "Window.hpp"

/**
 * \brief Represents a GUI static text
 */

class Text : public Window
{
		friend class GameFactory;

	private:
		void initialize();

		Text(const Text&) = delete;
		Text& operator=(const Text&) = delete;

	protected:
		Text();

		template<typename... Args>
		Text(Args&&... args) : Window(std::forward<Args>(args)...) { initialize(); }

		Text(pPacket& packet) : Text(const_cast<const pPacket&>(packet)) {}
		Text(const pPacket& packet);
		Text(pPacket&& packet) : Text(packet) {};

	public:
		virtual ~Text() noexcept;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(Text, Window, ID_TEXT)

PF_MAKE_E(ID_TEXT_NEW, pGeneratorReferenceExtend)

#endif
