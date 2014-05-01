#include "vaultscript.h"
#include "ilview.hpp"
#include "pawn.hpp"

#include <algorithm>

using namespace vaultmp;
using namespace std;

IDMultiHash<ID> item_to_list;
IDHash<ID> list_to_item;
IDHash<pair<string, bool>> format_cbs;
IDHash<string> notify_cbs;
IDMultiHash<ID> container_to_view;
IDHash<ID> view_to_container;

Result VAULTSCRIPT Create(ID id, cRawString notify, cRawString format) noexcept;

Void VAULTSCRIPT OnServerInit() noexcept
{
	MakePublic(Create, "IlView::Create");
}

Void VAULTSCRIPT OnServerExit(State error) noexcept
{
	IDVector views;
	transform(view_to_container.begin(), view_to_container.end(), back_inserter(views), [](const decltype(container_to_view)::value_type& list) { return list.first; });

	for_each(views.begin(), views.end(), [](ID list) {
		DestroyWindow(list);
	});
}

string GetFormat(ID list, ID item) noexcept
{
	const auto& cb = format_cbs[list];

	if (cb.second)
	{
		static string result;

		auto obtain = [](PAWN::cell* data) -> Result {
			vector<PAWN::cell> dest(IlView::MAX_LENGTH_ITEM + 1);
			PAWN::amx_StrUnpack(&dest[0], data, IlView::MAX_LENGTH_ITEM);

			const PAWN::cell* result_ = &dest[0];

			while (*result_)
				result.push_back(*result_++);

			return {};
		};

		if (result.empty())
			MakePublic(static_cast<Function<PAWN::cell*>>(obtain), "IlView::InternalObtain");
		else
			result.clear();

		CallPublic(cb.first, item, "IlView::InternalObtain");

		return result;
	}
	else
	{
		char result[IlView::MAX_LENGTH_ITEM + 1];
		CallPublic(cb.first, item, result);
		return result;
	}
}

ID CreateView(ItemList& itemlist, cRawString notify, cRawString format) noexcept
{
	ID list = List::Create(0.0, 0.0, 0.0, 0.0, True, True, "");

	notify_cbs.emplace(list, notify);
	format_cbs.emplace(list, make_pair(format, IsPAWN(format)));
	container_to_view.emplace(itemlist.GetID(), list);
	view_to_container.emplace(list, itemlist.GetID());

	auto items = itemlist.GetContainerItemList();

	for (const auto& id : items)
	{
		ID listitem = AddListItem(list, GetFormat(list, id));
		item_to_list.emplace(id, listitem);
		list_to_item.emplace(listitem, id);
	}

	return list;
}

Result VAULTSCRIPT Create(ID id, cRawString notify, cRawString format) noexcept
{
	ItemList itemlist(id);

	if (!itemlist || !notify || !format)
		return static_cast<Result>(False);

	return static_cast<Result>(CreateView(itemlist, notify, format));
}

Void VAULTSCRIPT OnCreate(ID id) noexcept
{
	Item item(id);
	ID container = item.GetItemContainer();

	auto range = container_to_view.equal_range(container);

	for_each(range.first, range.second, [id](const decltype(container_to_view)::value_type& list) {
		ID listitem = AddListItem(list.second, GetFormat(list.second, id));
		item_to_list.emplace(id, listitem);
		list_to_item.emplace(listitem, id);
	});
}

Void VAULTSCRIPT OnDestroy(ID id) noexcept
{
	auto range = item_to_list.equal_range(id);

	for_each(range.first, range.second, [id](const decltype(item_to_list)::value_type& listitem) {
		if (GetListItemSelected(listitem.second))
			CallPublic(notify_cbs[GetListItemContainer(listitem.second)], listitem.second, id, 0ull, False);

		RemoveListItem(listitem.second);
		list_to_item.erase(listitem.second);
	});

	item_to_list.erase(id);

	range = container_to_view.equal_range(id);
	IDVector views;
	transform(range.first, range.second, back_inserter(views), [](const decltype(container_to_view)::value_type& list) { return list.second; });

	for_each(views.begin(), views.end(), [](ID list) {
		DestroyWindow(list); // OnDestroy will be called for the list - below cleanup code will run
	});

	if (view_to_container.count(id))
	{
		auto listitems = GetListItemList(id);

		for (const auto& id : listitems)
		{
			item_to_list.erase(list_to_item[id]);
			list_to_item.erase(id);
		}

		container_to_view.erase(view_to_container[id]);
		view_to_container.erase(id);

		notify_cbs.erase(id);
		format_cbs.erase(id);
	}
}

Void VAULTSCRIPT OnItemCountChange(ID item, UCount count) noexcept
{
	auto range = item_to_list.equal_range(item);
	for_each(range.first, range.second, [item](const decltype(item_to_list)::value_type& listitem) {
		SetListItemText(listitem.second, GetFormat(GetListItemContainer(listitem.second), item));
	});
}

Void VAULTSCRIPT OnItemConditionChange(ID item, Value condition) noexcept
{
	auto range = item_to_list.equal_range(item);
	for_each(range.first, range.second, [item](const decltype(item_to_list)::value_type& listitem) {
		SetListItemText(listitem.second, GetFormat(GetListItemContainer(listitem.second), item));
	});
}

Void VAULTSCRIPT OnItemEquippedChange(ID item, State equipped) noexcept
{
	auto range = item_to_list.equal_range(item);
	for_each(range.first, range.second, [item](const decltype(item_to_list)::value_type& listitem) {
		SetListItemText(listitem.second, GetFormat(GetListItemContainer(listitem.second), item));
	});
}

Void VAULTSCRIPT OnListItemSelect(ID listitem, ID player, State selected) noexcept
{
	auto container = GetListItemContainer(listitem);

	if (notify_cbs.count(container))
		CallPublic(notify_cbs[container], container, list_to_item[listitem], player, selected);
}
