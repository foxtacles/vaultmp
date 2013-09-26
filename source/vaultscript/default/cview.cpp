#include "vaultscript.h"
#include "ilview.hpp"

using namespace vaultmp;
using namespace std;

struct ContainerView {
	ID il_left;
	ID il_right;
	ID button_left;
	ID button_right;
};

IDHash<ID> itemlist_to_view;
IDHash<ID> itemlist_to_itemlist;
IDHash<ID> button_to_view;
IDHash<IDSet> itemlist_sel;
IDHash<ContainerView> views;
IDHash<String> notify_cbs;

Result VAULTSCRIPT Create(ID itemlist_left, ID itemlist_right, cRawString notify, cRawString format) noexcept;

Void VAULTSCRIPT OnServerInit() noexcept
{
	MakePublic(Create, "CView::Create");
}

Result VAULTSCRIPT OnItemSelect(ID itemlist, ID item, ID, State selected) noexcept
{
	if (selected)
		itemlist_sel[itemlist].insert(item);
	else
		itemlist_sel[itemlist].erase(item);

	return static_cast<Result>(True);
}

Result VAULTSCRIPT Create(ID itemlist_left, ID itemlist_right, cRawString notify, cRawString format) noexcept
{
	if (itemlist_left == itemlist_right)
		return static_cast<Result>(False);

	auto il_left = IlView::Create(itemlist_left, OnItemSelect, format);

	if (!il_left)
		return static_cast<Result>(False);

	auto il_right = IlView::Create(itemlist_right, OnItemSelect, format);

	if (!il_right)
	{
		DestroyWindow(il_left);
		return static_cast<Result>(False);
	}

	SetWindowPos(il_left, 0.0, 0.0, 0.0, 0.0);
	SetWindowPos(il_right, 0.55, 0.0, 0.0, 0.0);
	SetWindowSize(il_left, 0.45, 1.0, 0.0, 0.0);
	SetWindowSize(il_right, 0.45, 1.0, 0.0, 0.0);

	SetListMultiSelect(il_left, True);
	SetListMultiSelect(il_right, True);

	auto main = Window::Create(0.4, 0.15, 0.57, 0.35, True, False, GetBaseName(itemlist_left) + " - " + GetBaseName(itemlist_right));
	auto button_left = Button::Create(0.47, 0.2, 0.06, 0.2, True, True, "<");
	auto button_right = Button::Create(0.47, 0.6, 0.06, 0.2, True, True, ">");

	AddChildWindow(main, il_left);
	AddChildWindow(main, il_right);
	AddChildWindow(main, button_left);
	AddChildWindow(main, button_right);

	itemlist_to_view.emplace(il_left, main);
	itemlist_to_view.emplace(il_right, main);
	itemlist_to_itemlist.emplace(il_left, itemlist_left);
	itemlist_to_itemlist.emplace(il_right, itemlist_right);
	button_to_view.emplace(button_left, main);
	button_to_view.emplace(button_right, main);
	views.emplace(main, ContainerView{il_left, il_right, button_left, button_right});
	notify_cbs.emplace(main, notify);

	return static_cast<Result>(main);
}

Void VAULTSCRIPT OnDestroy(ID reference) noexcept
{
	views.erase(reference);
	notify_cbs.erase(reference);
	itemlist_to_itemlist.erase(reference);
	itemlist_sel.erase(reference);
	button_to_view.erase(reference);

	if (itemlist_to_view.count(reference))
	{
		auto main = itemlist_to_view[reference];
		itemlist_to_view.erase(reference);
		DestroyWindow(main);
	}
}

Void VAULTSCRIPT OnWindowClick(ID window, ID player) noexcept
{
	if (button_to_view.count(window))
	{
		const auto& view = views[button_to_view[window]];

		if (view.button_left == window)
		{
			for (auto item : itemlist_sel[view.il_right])
				if (CallPublic(notify_cbs[button_to_view[window]], button_to_view[window], item, itemlist_to_itemlist[view.il_left], player))
					SetItemContainer(item, itemlist_to_itemlist[view.il_left]);
		}
		else
			for (auto item : itemlist_sel[view.il_left])
				if (CallPublic(notify_cbs[button_to_view[window]], button_to_view[window], item, itemlist_to_itemlist[view.il_right], player))
					SetItemContainer(item, itemlist_to_itemlist[view.il_right]);
	}
}
