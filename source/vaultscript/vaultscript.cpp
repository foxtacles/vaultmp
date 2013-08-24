#include "vaultscript.h"
#include "default/pickup.hpp"
#include "default/convo.hpp"

#include <cstdio>

using namespace vaultmp;

Result VAULTSCRIPT OnItemPickup(ID item, ID actor) noexcept;

ID win;
ID grp;
ID it;

Void VAULTSCRIPT OnServerInit() noexcept
{
	std::printf("My first C++ vaultscript <3\n");
	SetServerName("vaultmp 0.1a server");
	SetServerRule("website", "vaultmp.com");
	SetServerMap("the wasteland");

	Pickup::Register(OnItemPickup, "vaultscript::OnItemPickup");

	win = Window::Create(0.3, 0.3, 0.7, 0.7, True, False, "Some window");

	Window window(win);
	window.AddChildWindow(Edit::Create(0.0, 0.0, 1.0, 0.1, True, False, "Some edit"));
	window.AddChildWindow(Button::Create(0.0, 0.1, 1.0, 0.1, True, False, "Some button"));
	window.AddChildWindow(Text::Create(0.0, 0.2, 1.0, 0.1, True, False, "Some text"));
	window.AddChildWindow(Checkbox::Create(0.0, 0.3, 1.0, 0.1, True, False, "Some checkbox"));

	ID r1 = RadioButton::Create(0.0, 0.4, 0.25, 0.1, True, False, "R1");
	ID r2 = RadioButton::Create(0.25, 0.4, 0.25, 0.1, True, False, "R2");
	ID r3 = RadioButton::Create(0.5, 0.4, 0.25, 0.1, True, False, "R3");
	ID r4 = RadioButton::Create(0.75, 0.4, 0.25, 0.1, True, False, "R4");

	SetRadioButtonGroup(r1, 0);
	SetRadioButtonGroup(r2, 0);
	SetRadioButtonGroup(r3, 1);
	SetRadioButtonGroup(r4, 1);

	grp =r3;

	window.AddChildWindow(r1);
	window.AddChildWindow(r2);
	window.AddChildWindow(r3);
	window.AddChildWindow(r4);

	ID list = List::Create(0.0, 0.5, 1.0, 0.2, True, False, "Some list");
	AddListItem(list, "bla1");
	AddListItem(list, "bla2");
	it = AddListItem(list, "bla3");
	AddListItem(list, "bla1");
	AddListItem(list, "bla2");
	AddListItem(list, "bla3");
	AddListItem(list, "bla1");
	AddListItem(list, "bla2");
	AddListItem(list, "bla3");
	window.AddChildWindow(list);
}

Void VAULTSCRIPT OnServerExit() noexcept
{

}

Void VAULTSCRIPT OnGameYearChange(UCount year) noexcept
{

}

Void VAULTSCRIPT OnGameMonthChange(UCount month) noexcept
{

}

Void VAULTSCRIPT OnGameDayChange(UCount day) noexcept
{

}

Void VAULTSCRIPT OnGameHourChange(UCount hour) noexcept
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
#include <cstring>
State VAULTSCRIPT OnPlayerChat(ID player, RawString message) noexcept
{
	if (!std::strcmp(message, "fatman")) {

			Player q(player);
			Value X,Y,Z;
			q.GetPos(X,Y,Z);

			Item::Create(WEAP::WeapFatman, q.GetCell(), X, Y, Z + 150);
	return False;
	}
	else if (!std::strcmp(message,"guy")) {

			Player q(player);
			Value X,Y,Z;
			q.GetPos(X,Y,Z);

			auto p = Actor::Create(NPC_::Carter, q.GetCell(), X, Y, Z + 400);

			AddItem(p, ARMO::VaultSuit101);
	return False;
	}
	else if (!std::strcmp(message,"kill")) {

		auto a = Actor::GetList();

		for (auto w : a)
		{

KillActor(w, w, Limb::ALL_LIMBS);
		}

	return False;
	}
	else if (!std::strcmp(message,"aggr")) {

		auto a = Actor::GetList();
			Player q(player);
			Value X,Y,Z;
			q.GetPos(X,Y,Z);
		for (auto w : a)
		{


			SetCell(w,q.GetCell(), X, Y, Z );
		}

	return False;
	}
	else if (!std::strcmp(message,"boom")) {

		Volatile<PROJ> ww(PROJ::FatMan, -90,0,0);
	ww.Create(player);

	return False;
	}
	else if (!std::strcmp(message,"expl")) {

		Volatile<EXPL> ww(EXPL::EyebotExplosion, -90,0,0);
	ww.Create(player);

	return False;
	}

	else if (!std::strcmp(message,"fire")) {

		SetItemEquipped(AddItem(player, WEAP::WeapFlamer), True);
		AddItem(player, AMMO::AmmoFlamerFuel, 500);

	return False;
	}

	else if (!std::strcmp(message,"mekill")) {

		KillActor(player,(ID) 0);

	return False;
	}
	else if (!std::strcmp(message,"convo")) {

	auto w=	Player::GetList();

		for (auto q : w)
		{
			if (q != player) {
			Conversation::Open(player,q);
			}
		}

	return False;
	}
	else if (!std::strcmp(message,"weap")) {

		SetItemEquipped(AddItem(player, WEAP::Weap10mmSubmachineGun), True);
		AddItem(player, AMMO::Ammo10mm, 500);

	return False;
	}
	else if (!std::strcmp(message,"grp")) {

		SetRadioButtonGroup(grp, 0);

	return False;
	}
	else if (!std::strcmp(message,"it")) {

		SetListItemSelected(it, True);
SetListItemText(it, "blub!");

	return False;
	}
	else if (!std::strcmp(message,"rem")) {

		RemoveListItem(it);

	return False;
	}

	return True;
}

Void VAULTSCRIPT OnWindowMode(ID player, State enabled) noexcept
{

}

Void VAULTSCRIPT OnWindowClick(ID window, ID player) noexcept
{

}

Void VAULTSCRIPT OnWindowTextChange(ID window, ID player, cRawString text) noexcept
{

}

Void VAULTSCRIPT OnCheckboxSelect(ID checkbox, ID player, State selected) noexcept
{
	printf("CB %llu %d\n", checkbox, selected);
}

Void VAULTSCRIPT OnRadioButtonSelect(ID radiobutton, ID previous, ID player) noexcept
{
	printf("RB %llu %llu\n", radiobutton, previous);
}

Void VAULTSCRIPT OnListItemSelect(ID listitem, ID player, State selected) noexcept
{
	printf("LI %llu %d\n", listitem, selected);
}

Void VAULTSCRIPT OnCreate(ID reference) noexcept
{

}

Void VAULTSCRIPT OnDestroy(ID reference) noexcept
{

}

Void VAULTSCRIPT OnSpawn(ID object) noexcept
{
	Player player(object);

	player.AttachWindow(win);

	if (player)
		player.UIMessage("Hello, " + player.GetBaseName() + "!");
}

Void VAULTSCRIPT OnActivate(ID object, ID actor) noexcept
{

}

Void VAULTSCRIPT OnCellChange(ID object, CELL cell) noexcept
{

}

Void VAULTSCRIPT OnLockChange(ID object, ID actor, Lock lock) noexcept
{

}

Result VAULTSCRIPT OnItemPickup(ID item, ID actor) noexcept
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
