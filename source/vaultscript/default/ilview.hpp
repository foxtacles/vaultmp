#include "vaultscript.h"

#include <cstdio>

namespace vaultmp
{
	namespace IlView
	{
		constexpr unsigned int MAX_LENGTH_ITEM = 64;

		VAULTFUNCTION ID Create(ID itemlist, Function<ID, ID, ID, State> notify, Function<ID, char*> format) noexcept {
			char function_name[16];

			std::snprintf(function_name, sizeof(function_name), "%p", reinterpret_cast<void*>(notify));
			String notify_(function_name);
			MakePublic(notify, notify_);

			std::snprintf(function_name, sizeof(function_name), "%p", reinterpret_cast<void*>(format));
			String format_(function_name);
			MakePublic(format, format_);

			return static_cast<ID>(CallPublic("IlView::Create", itemlist, notify_.c_str(), format_.c_str()));
		}

		VAULTFUNCTION ID Create(ID itemlist, const String& notify_, Function<ID, char*> format) noexcept {
			char function_name[16];

			std::snprintf(function_name, sizeof(function_name), "%p", reinterpret_cast<void*>(format));
			String format_(function_name);
			MakePublic(format, format_);

			return static_cast<ID>(CallPublic("IlView::Create", itemlist, notify_.c_str(), format_.c_str()));
		}

		VAULTFUNCTION ID Create(ID itemlist, Function<ID, ID, ID, State> notify, const String& format_) noexcept {
			char function_name[16];

			std::snprintf(function_name, sizeof(function_name), "%p", reinterpret_cast<void*>(notify));
			String notify_(function_name);
			MakePublic(notify, notify_);

			return static_cast<ID>(CallPublic("IlView::Create", itemlist, notify_.c_str(), format_.c_str()));
		}

		VAULTFUNCTION ID Create(ID itemlist, const String& notify_, const String& format_) noexcept {
			return static_cast<ID>(CallPublic("IlView::Create", itemlist, notify_.c_str(), format_.c_str()));
		}
	}
}
