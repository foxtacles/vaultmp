#include <vaultmp>

stock CView_Create(itemlist_left, itemlist_right, const notify{}, const notify_name{}, const format{}, const format_name{}) {
	MakePublic(notify, notify_name, "llll");
	MakePublic(format, format_name, "ls");
	return CallPublic("CView::Create", itemlist_left, itemlist_right, notify_name, format_name);
}
