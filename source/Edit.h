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
		static constexpr unsigned int DEFAULT_LENGTH = UINT_MAX;
		static constexpr const char* DEFAULT_VALIDATION = ".*";

		unsigned int length;
		std::string validation;

		void initialize();

		Edit(const Edit&);
		Edit& operator=(const Edit&);

	protected:
		Edit();
		Edit(const pDefault* packet);
		Edit(pPacket&& packet);

	public:
		virtual ~Edit();

		void SetMaxLength(unsigned int length) { this->length = length; }
		void SetValidation(const std::string& validation) { this->validation = validation; }

		unsigned int GetMaxLength() { return length; }
		const std::string& GetValidation() { return validation; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#endif
