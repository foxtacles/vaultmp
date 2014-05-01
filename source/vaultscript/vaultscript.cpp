#include "vaultscript.h"
#include "default/pickup.hpp"
#include "default/ilview.hpp"
#include "default/cview.hpp"

#include <cstdio>

using namespace vaultmp;

struct TestWindow {
	ID male;
	ID female;
	ID make_guy;
	ID make_fatman;
	ID drink_beer;
	ID give_10mm;
	ID give_flamer;
	ID spawn_nuke;
	ID spawn_expl;
	ID all_guys;
	ID kill_guys;
};

IDHash<ID> open_cviews;
IDHash<ID> cview_map;
IDHash<ID> player_test_windows;
IDHash<TestWindow> test_windows;

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

Result VAULTSCRIPT Timer_RemoveActor(ID actor) noexcept
{
	DestroyObject(actor);
	KillTimer();
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

Void MakeTestWindow(ID player) noexcept
{
	ID window = Window::Create(0.7, 0.3, 0.3, 0.5, True, False, "vaultscript test");

	TestWindow data{
		RadioButton::Create(0.0, 0.0, 0.5, 0.1, True, True, "Male"),
		RadioButton::Create(0.5, 0.0, 0.5, 0.1, True, True, "Female"),
		Button::Create(0.0, 0.1, 1.0, 0.1, True, True, "Make guy"),
		Button::Create(0.0, 0.2, 1.0, 0.1, True, True, "Make fatman"),
		Button::Create(0.0, 0.3, 1.0, 0.1, True, True, "Drink beer"),
		Button::Create(0.0, 0.4, 1.0, 0.1, True, True, "Give 10mm"),
		Button::Create(0.0, 0.5, 1.0, 0.1, True, True, "Give flamer"),
		Button::Create(0.0, 0.6, 1.0, 0.1, True, True, "Spawn nuke"),
		Button::Create(0.0, 0.7, 1.0, 0.1, True, True, "Fire me"),
		Button::Create(0.0, 0.8, 1.0, 0.1, True, True, "All guys to me"),
		Button::Create(0.0, 0.9, 1.0, 0.1, True, True, "Kill guys")
	};

	SetRadioButtonGroup(data.male, GetConnection(player));
	SetRadioButtonGroup(data.female, GetConnection(player));

	if (GetActorBaseSex(player) == Sex::Male)
		SetRadioButtonSelected(data.male, True);
	else
		SetRadioButtonSelected(data.female, True);

	AddChildWindow(window, data.male);
	AddChildWindow(window, data.female);
	AddChildWindow(window, data.make_guy);
	AddChildWindow(window, data.make_fatman);
	AddChildWindow(window, data.drink_beer);
	AddChildWindow(window, data.give_10mm);
	AddChildWindow(window, data.give_flamer);
	AddChildWindow(window, data.spawn_nuke);
	AddChildWindow(window, data.spawn_expl);
	AddChildWindow(window, data.all_guys);
	AddChildWindow(window, data.kill_guys);

	AttachWindow(player, window);

	player_test_windows.emplace(player, window);
	test_windows.emplace(window, data);
}

Void VAULTSCRIPT OnServerInit() noexcept
{
	std::printf("My first C++ vaultscript <3\n");
	SetServerName("vaultmp 0.1a server");
	SetServerRule("website", "vaultmp.com");
	SetServerMap("the wasteland");

	Pickup::Register(OnItemPickup);
}

Void VAULTSCRIPT OnServerExit(State error) noexcept
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
	if (player_test_windows[player])
		DestroyWindow(player_test_windows[player]);
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
	ID parent = GetWindowParent(window);

	if (test_windows.count(parent))
	{
		const auto& test_window = test_windows[parent];

		if (window == test_window.make_guy)
		{
			Value X, Y, Z;
			GetPos(player, X, Y, Z);

			Actor::Create(NPC_::Carter, GetCell(player), X, Y, Z + 400);
		}
		else if (window == test_window.make_fatman)
		{
			Value X, Y, Z;
			GetPos(player, X, Y, Z);

			Item::Create(WEAP::WeapFatman, GetCell(player), X, Y, Z + 150);
		}
		else if (window == test_window.drink_beer)
			PlayIdle(player, IDLE::BeerDrinkingStanding);
		else if (window == test_window.give_10mm)
		{
			auto items = GetContainerItemList(player);

			for (auto item : items)
				if (GetItemEquipped(item) && !BaseToType(GetBase(item)).compare("WEAP"))
					SetItemEquipped(item, False);

			SetItemEquipped(AddItem(player, WEAP::Weap10mmSubmachineGun), True);
			AddItem(player, AMMO::Ammo10mm, 500);
		}
		else if (window == test_window.give_flamer)
		{
			auto items = GetContainerItemList(player);

			for (auto item : items)
				if (GetItemEquipped(item) && !BaseToType(GetBase(item)).compare("WEAP"))
					SetItemEquipped(item, False);

			SetItemEquipped(AddItem(player, WEAP::WeapFlamer), True);
			AddItem(player, AMMO::AmmoFlamerFuel, 500);
		}
		else if (window == test_window.spawn_nuke)
			CreateVolatile(player, PROJ::FatMan, -90.0, 0.0, 0.0);
		else if (window == test_window.spawn_expl)
			CreateVolatile(player, EXPL::ExplosionFlamerCritEffect, 0.0, 0.0, 0.0);
		else if (window == test_window.all_guys)
		{
			auto actors = Actor::GetList();

			Value X, Y, Z;
			GetPos(player, X, Y, Z);

			for (auto actor : actors)
				SetCell(actor, GetCell(player), X, Y, Z);
		}
		else if (window == test_window.kill_guys)
		{
			auto actors = Actor::GetList();

			for (auto actor : actors)
				KillActor(actor, actor, Limb::ALL_LIMBS);
		}

		ForceWindowMode(player, False);
	}
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
	ID parent = GetWindowParent(radiobutton);

	if (test_windows.count(parent))
	{
		const auto& test_window = test_windows[parent];

		if (test_window.male == radiobutton)
			SetActorBaseSex(player, Sex::Male);
		else
			SetActorBaseSex(player, Sex::Female);
	}
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
	test_windows.erase(reference);
}

Void VAULTSCRIPT OnSpawn(ID object) noexcept
{
	Player player(object);

	if (player)
	{
		player.UIMessage("Hello, " + player.GetBaseName() + "!");

		if (!player_test_windows.count(object))
			MakeTestWindow(object);
	}
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
	constexpr Interval delete_after_ms = static_cast<Interval>(10000);

	if (IsActor(actor))
		CreateTimerEx(Timer_RemoveActor, delete_after_ms, actor);
}

Void VAULTSCRIPT OnActorPunch(ID actor, State power) noexcept
{

}

Void VAULTSCRIPT OnActorFireWeapon(ID actor, WEAP weapon) noexcept
{

}
