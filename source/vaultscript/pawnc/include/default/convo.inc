#include <vaultmp>

stock Conversation_Open(ID, with_id) {
	return CallPublic("Conversation::Open", ID, with_id);
}

stock Conversation_Toggle(ID, with_id, Bool:toggle) {
	return CallPublic("Conversation::Toggle", ID, with_id, toggle);
}

stock Conversation_Close(ID, with_id) {
	return CallPublic("Conversation::Close", ID, with_id);
}
