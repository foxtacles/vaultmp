#ifndef EDITGUI_H
#define EDITGUI_H

#include "vaultmp.hpp"
#include "Window.hpp"

#include <string>
#include <climits>

/**
 * \brief Represents a GUI edit
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

		Edit(const Edit&) = delete;
		Edit& operator=(const Edit&) = delete;

	protected:
		Edit();

		template<typename... Args>
		Edit(Args&&... args) : Window(std::forward<Args>(args)...), length(DEFAULT_LENGTH), validation(DEFAULT_VALIDATION) { initialize(); }

		Edit(pPacket& packet) : Edit(const_cast<const pPacket&>(packet)) {}
		Edit(const pPacket& packet);
		Edit(pPacket&& packet) : Edit(packet) {};

	public:
		virtual ~Edit() noexcept;

		void SetMaxLength(unsigned int length) { this->length = length; }
		void SetValidation(const std::string& validation) { this->validation = validation; }

		unsigned int GetMaxLength() const { return length; }
		const std::string& GetValidation() const { return validation; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER_FINAL(Edit, Window, ID_EDIT)

PF_MAKE(ID_EDIT_NEW, pGeneratorReferenceExtend, unsigned int, std::string)
PF_MAKE(ID_UPDATE_WMAXLEN, pGeneratorReference, unsigned int)
PF_MAKE(ID_UPDATE_WVALID, pGeneratorReference, std::string)

#endif
