#ifndef BUTTONGUI_H
#define BUTTONGUI_H

#include <string>

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
		Button(pPacket&& packet);

	public:
		virtual ~Button();

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#endif
