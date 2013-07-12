#ifndef TEXTGUI_H
#define TEXTGUI_H

#include <string>

#include "vaultmp.h"
#include "Window.h"

/**
 * \brief Represents a GUI static text
 */

class Text : public Window
{
		friend class GameFactory;

	private:
		std::string text;

		void initialize();

		Text(const Text&);
		Text& operator=(const Text&);

	protected:
		Text();
		Text(const pDefault* packet);
		Text(pPacket&& packet);

	public:
		virtual ~Text();

		void SetText(const std::string& text) { this->text = text; }

		const std::string& GetText() const { return text; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#endif
