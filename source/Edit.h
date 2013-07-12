#ifndef EDITGUI_H
#define EDITGUI_H

#include <string>

#include "vaultmp.h"
#include "Window.h"

/**
 * \brief Represents a GUI static text
 */

class Edit : public Window
{
		friend class GameFactory;

	private:
		void initialize();

		Edit(const Edit&);
		Edit& operator=(const Edit&);

	protected:
		Edit();
		Edit(const pDefault* packet);
		Edit(pPacket&& packet);

	public:
		virtual ~Edit();

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#endif
