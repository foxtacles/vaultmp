#include "vaultscript.h"
#include "default/pickup.hpp"
#include "default/ilview.hpp"
#include "default/cview.hpp"

#include <cstdio>

using namespace vaultmp;

IDHash<ID> open_cviews;
IDHash<ID> cview_map;

Result VAULTSCRIPT OnItemPickup(ID item, ID actor) noexcept;
Result VAULTSCRIPT OnItemMove(ID cview, ID item, ID destination, ID player) noexcept;

Result VAULTSCRIPT FormatItemCView(ID item, char* format) noexcept
{
	Item data(item);
	Actor actor(data.GetItemContainer());
	auto type = data.BaseToType();

	if (!type.compare("WEAP") || !type.compare("ARMO") || !type.compare("ARMA"))
		std::snprintf(format, IlView::MAX_LENGTH_ITEM, "%dx %s (%d%%)%s", data.GetItemCount(), data.BaseToString().c_str(), static_cast<unsigned int>(data.GetItemCondition()), actor && data.GetItemEquipped() ? " (equipped)" : "");
	else
		std::snprintf(format, IlView::MAX_LENGTH_ITEM, "%dx %s", data.GetItemCount(), data.BaseToString().c_str());

	return static_cast<Result>(True);
}

Result VAULTSCRIPT Timer_CloseCView(ID player, ID cview) noexcept
{
	constexpr Value distance = 150.0;

	Container container(cview_map[cview]);
	Value X, Y, Z;
	container.GetPos(X, Y, Z);

	if (!container || !IsNearPoint(player, X, Y, Z, distance))
	{
		DestroyWindow(cview);
		KillTimer();
	}

	return static_cast<Result>(True);
}

State IsCViewOpen(ID player, ID itemlist) noexcept
{
	for (const auto& cview : open_cviews)
		if (cview.second == player)
			if (cview_map[cview.first] == itemlist)
				return True;

	return False;
}

Void VAULTSCRIPT OnServerInit() noexcept
{
	std::printf("My first C++ vaultscript <3\n");
	SetServerName("vaultmp 0.1a server");
	SetServerRule("website", "vaultmp.com");
	SetServerMap("the wasteland");

	Pickup::Register(OnItemPickup);
}

Void VAULTSCRIPT OnServerExit() noexcept
{

}

Void VAULTSCRIPT OnGameTimeChange(UCount year, UCount month, UCount day, UCount hour) noexcept
{

}

State VAULTSCRIPT OnClientAuthenticate(cRawString name, cRawString pwd) noexcept
{
	return True;
}

Void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason) noexcept
{

}

NPC_ VAULTSCRIPT OnPlayerRequestGame(ID player) noexcept
{
	return static_cast<NPC_>(0x00000000);
}

State VAULTSCRIPT OnPlayerChat(ID player, RawString message) noexcept
{
	return True;
}

Void VAULTSCRIPT OnWindowMode(ID player, State enabled) noexcept
{

}

Void VAULTSCRIPT OnWindowClick(ID window, ID player) noexcept
{

}

Void VAULTSCRIPT OnWindowReturn(ID window, ID player) noexcept
{

}

Void VAULTSCRIPT OnWindowTextChange(ID window, ID player, cRawString text) noexcept
{

}

Void VAULTSCRIPT OnCheckboxSelect(ID checkbox, ID player, State selected) noexcept
{

}

Void VAULTSCRIPT OnRadioButtonSelect(ID radiobutton, ID previous, ID player) noexcept
{

}

Void VAULTSCRIPT OnListItemSelect(ID listitem, ID player, State selected) noexcept
{

}

Void VAULTSCRIPT OnCreate(ID reference) noexcept
{

}

Void VAULTSCRIPT OnDestroy(ID reference) noexcept
{
	open_cviews.erase(reference);
	cview_map.erase(reference);
}

Void VAULTSCRIPT OnSpawn(ID object) noexcept
{
	Player player(object);

	if (player)
		player.UIMessage("Hello, " + player.GetBaseName() + "!");
}

Void VAULTSCRIPT OnActivate(ID object, ID actor) noexcept
{
	Container container(object);
	Player player(actor);

	if (container && player && container.GetLock() == Lock::Unlocked && !IsCViewOpen(actor, object))
	{
		auto cview = CView::Create(actor, object, OnItemMove, FormatItemCView);
		open_cviews.emplace(cview, actor);
		cview_map.emplace(cview, object);
		player.AttachWindow(cview);

		constexpr Interval interval = static_cast<Interval>(500);
		CreateTimerEx(Timer_CloseCView, interval, actor, cview);
	}
}

Void VAULTSCRIPT OnCellChange(ID object, CELL cell) noexcept
{

}

Void VAULTSCRIPT OnLockChange(ID object, ID actor, Lock lock) noexcept
{

}

Void VAULTSCRIPT OnItemCountChange(ID item, UCount count) noexcept
{

}

Void VAULTSCRIPT OnItemConditionChange(ID item, Value condition) noexcept
{

}

Void VAULTSCRIPT OnItemEquippedChange(ID item, State equipped) noexcept
{

}

Result VAULTSCRIPT OnItemPickup(ID item, ID actor) noexcept
{
	return static_cast<Result>(True);
}

Result VAULTSCRIPT OnItemMove(ID cview, ID item, ID destination, ID player) noexcept
{
	return static_cast<Result>(True);
}

Void VAULTSCRIPT OnActorValueChange(ID actor, ActorValue index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorBaseValueChange(ID actor, ActorValue index, Value value) noexcept
{

}

Void VAULTSCRIPT OnActorAlert(ID actor, State alerted) noexcept
{

}

Void VAULTSCRIPT OnActorSneak(ID actor, State sneaking) noexcept
{

}

Void VAULTSCRIPT OnActorDeath(ID actor, ID killer, Limb limbs, Death cause) noexcept
{

}

Void VAULTSCRIPT OnActorPunch(ID actor, State power) noexcept
{

}

Void VAULTSCRIPT OnActorFireWeapon(ID actor, WEAP weapon) noexcept
{

}
