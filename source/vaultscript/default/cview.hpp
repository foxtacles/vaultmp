#include "vaultscript.h"

#include <cstdio>

namespace vaultmp
{
	namespace CView
	{
		VAULTFUNCTION ID Create(ID itemlist_left, ID itemlist_right, Function<ID, ID, ID, ID> notify, Function<ID, char*> format) noexcept {
			char function_name[16];

			std::snprintf(function_name, sizeof(function_name), "%p", reinterpret_cast<void*>(notify));
			String notify_(function_name);
			MakePublic(notify, notify_);

			std::snprintf(function_name, sizeof(function_name), "%p", reinterpret_cast<void*>(format));
			String format_(function_name);
			MakePublic(format, format_);

			return static_cast<ID>(CallPublic("CView::Create", itemlist_left, itemlist_right, notify_.c_str(), format_.c_str()));
		}
	}
}
