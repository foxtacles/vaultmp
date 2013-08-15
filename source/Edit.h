#ifndef EDITGUI_H
#define EDITGUI_H

#include "vaultmp.h"
#include "Window.h"

#include <string>

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

		Edit(const Edit&);
		Edit& operator=(const Edit&);

	protected:
		Edit();
		Edit(const pDefault* packet);
		Edit(pPacket&& packet) : Edit(packet.get()) {};

	public:
		virtual ~Edit() noexcept;

		void SetMaxLength(unsigned int length) { this->length = length; }
		void SetValidation(const std::string& validation) { this->validation = validation; }

		unsigned int GetMaxLength() { return length; }
		const std::string& GetValidation() { return validation; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER(Edit, Window, ID_EDIT)

template<> struct pTypesMap<pTypes::ID_EDIT_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_EDIT_NEW, unsigned int, std::string> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WMAXLEN> { typedef pGeneratorReference<pTypes::ID_UPDATE_WMAXLEN, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WVALID> { typedef pGeneratorReference<pTypes::ID_UPDATE_WVALID, std::string> type; };

#endif
