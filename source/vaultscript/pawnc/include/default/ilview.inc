#include <vaultmp>

const IlView: {
	MAX_LENGTH_ITEM = 64
}

stock IlView_Create(ID, const notify{}, const notify_name{}, const format{}, const format_name{}) {
	MakePublic(notify, notify_name, "llli");
	MakePublic(format, format_name, "ls");
	return CallPublic("IlView::Create", ID, notify_name, format_name);
}
