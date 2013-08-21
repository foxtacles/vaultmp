#include "vaultscript.h"

#include <tuple>

using namespace vaultmp;
using namespace std;

tuple<double, double, double, double> convo_root{0.4, 0.4, 0.3, 0.3};
tuple<double, double, double, double> convo_text{0.0, 0.0, 1.0, 0.7};
tuple<double, double, double, double> convo_edit{0.0, 0.7, 0.7, 0.3};
tuple<double, double, double, double> convo_button{0.7, 0.7, 0.3, 0.3};

struct ConversationWindow {
	ID root;
	ID text;
	ID edit;
	ID button;
};

struct Conversation {
	ID with_id;
	string text;
	ConversationWindow window;
};

std::unordered_multimap<ID, Conversation, _hash_ID> conversations;
Result VAULTSCRIPT Open(ID id, ID with_id) noexcept;
Result VAULTSCRIPT Toggle(ID id, ID with_id, State toggle) noexcept;
Result VAULTSCRIPT Close(ID id, ID with_id) noexcept;

decltype(conversations)::iterator FindConversation(ID id, ID with_id)
{
	auto range = conversations.equal_range(id);

	while (range.first != range.second)
	{
		if (range.first->second.with_id == with_id)
			return range.first;

		++range.first;
	}

	return conversations.end();
}

Result VAULTSCRIPT Open(ID id, ID with_id) noexcept
{
	Player player1(id);
	Player player2(with_id);

	if (!player1 || !player2 || player1 == player2 || FindConversation(id, with_id) != conversations.end())
		return static_cast<Result>(False);

	Conversation convo;
	convo.with_id = with_id;
	convo.window.root = Window::Create(get<0>(convo_root), get<1>(convo_root), 0.0, 0.0, get<2>(convo_root), get<3>(convo_root), 0.0, 0.0, True, False, String("Conversation with " + player2.GetBaseName()));
	convo.window.text = Text::Create(get<0>(convo_text), get<1>(convo_text), 0.0, 0.0, get<2>(convo_text), get<3>(convo_text), 0.0, 0.0, True, False, "");
	convo.window.edit = Edit::Create(get<0>(convo_edit), get<1>(convo_edit), 0.0, 0.0, get<2>(convo_edit), get<3>(convo_edit), 0.0, 0.0, True, False, "");
	convo.window.button = Button::Create(get<0>(convo_button), get<1>(convo_button), 0.0, 0.0, get<2>(convo_button), get<3>(convo_button), 0.0, 0.0, True, False, "Send");

	AddChildWindow(convo.window.root, convo.window.text);
	AddChildWindow(convo.window.root, convo.window.edit);
	AddChildWindow(convo.window.root, convo.window.button);

	conversations.emplace(id, move(convo));
	AttachWindow(id, convo.window.root);

	return static_cast<Result>(True);
}

Result VAULTSCRIPT Toggle(ID id, ID with_id, State toggle) noexcept
{
	auto convo = FindConversation(id, with_id);

	if (convo == conversations.end())
		return static_cast<Result>(False);

	return static_cast<Result>(SetWindowVisible(convo->second.window.root, toggle));
}

Result VAULTSCRIPT Close(ID id, ID with_id) noexcept
{
	auto convo = FindConversation(id, with_id);

	if (convo == conversations.end())
		return static_cast<Result>(False);

	DestroyWindow(convo->second.window.root);
	conversations.erase(convo);

	return static_cast<Result>(True);
}

Void VAULTSCRIPT OnServerInit() noexcept
{
	MakePublic(Open, "Conversation::Open");
	MakePublic(Toggle, "Conversation::Toggle");
	MakePublic(Close, "Conversation::Close");
}

Void VAULTSCRIPT OnServerExit() noexcept
{
	for (const auto& convo : conversations)
		DestroyWindow(convo.second.window.root);

	conversations.clear();
}

Void VAULTSCRIPT OnWindowClick(ID window, ID player) noexcept
{
	auto range = conversations.equal_range(player);

	while (range.first != range.second)
	{
		auto& data = *range.first;
		auto& window_convo = data.second.window;

		if (window_convo.button == window)
		{
			auto text = GetWindowText(window_convo.edit);

			if (!text.empty())
			{
				data.second.text += (data.second.text.empty() ? "" : "\n") + GetBaseName(player) + ": " + text;
				SetWindowText(window_convo.edit, "");
				SetWindowText(window_convo.text, data.second.text);

				auto with_id = FindConversation(data.second.with_id, player);

				if (with_id != conversations.end())
				{
					with_id->second.text = data.second.text;
					SetWindowText(with_id->second.window.text, data.second.text);
				}
			}

			break;
		}

		++range.first;
	}
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason) noexcept
{
	auto it = conversations.begin();

	while (it != conversations.end())
	{
		if (it->first == player || it->second.with_id == player)
		{
			DestroyWindow(it->second.window.root);
			conversations.erase(it++);
			continue;
		}

		++it;
	}
}
