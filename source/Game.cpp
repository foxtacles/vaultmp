#include "Game.h"
#include "Shared.h"

#include <algorithm>

using namespace std;
using namespace RakNet;
using namespace Values;

RakNetGUID Game::server;

Guarded<Game::CellRefs> Game::cellRefs;
Guarded<Player::CellContext> Game::cellContext;
Guarded<Game::UninitializedObjects> Game::uninitObj;
Guarded<Game::DeletedObjects> Game::deletedObj;
Guarded<Game::DeletedObjects> Game::deletedStatic;
Game::BaseRaces Game::baseRaces;
Game::Globals Game::globals;
Game::Weather Game::weather;
Game::PlayerBase Game::playerBase;
Game::SpawnFunc Game::spawnFunc;
Player::CellContext Game::spawnContext;
Game::StartupQueue Game::startupQueue;
bool Game::GUIMode;
bool Game::startup;

#ifdef VAULTMP_DEBUG
DebugInput<Game> Game::debug;
#endif

void Game::AdjustZAngle(double& Z, double diff)
{
	Z += diff;

	if (Z > 360.0)
		Z -= 360.0;
	else if (Z < 0.00)
		Z += 360.0;
}

void Game::CommandHandler(unsigned int key, const vector<double>& info, double result, bool error)
{
	Func opcode = getFrom<Func>(info.at(0));

	if (!error)
	{
#ifdef VAULTMP_DEBUG
		//debug.print("Executing command ", hex, opcode, " on reference ", info.size() > 1 ? getFrom<unsigned int>(info.at(1)) : 0);
#endif

		weak_ptr<Lockable> shared;

		if (key)
		{
			switch (opcode)
			{
				case Func::CenterOnCell:
				case Func::CenterOnWorld:
				case Func::ForceRespawn:
				case Func::PlaceAtMeHealthPercent:
					shared = Lockable::Poll(key);
					break;

				default:
					Lockable::Retrieve(key);
			}
		}

		switch (opcode)
		{
			case Func::PlaceAtMeHealthPercent:
				FutureSet(shared, getFrom<unsigned int>(result));
				break;

			case Func::GetPos:
			{
				auto reference = GameFactory::GetObject(getFrom<unsigned int>(info.at(1)));
				GetPos(reference.get(), getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func::SetPos:
				break;

			case Func::GetAngle:
			{
				auto reference = GameFactory::GetObject(getFrom<unsigned int>(info.at(1)));
				GetAngle(reference.get(), getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func::SetAngle:
				break;

			case Func::GetActorValue:
			{
				auto reference = GameFactory::GetObject<Actor>(getFrom<unsigned int>(info.at(1)));
				//GetActorValue(reference.get(), false, getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func::ForceActorValue:
				break;

			case Func::DamageActorValue:
				break;

			case Func::RestoreActorValue:
				break;

			case Func::GetBaseActorValue:
			{
				auto reference = GameFactory::GetObject<Actor>(getFrom<unsigned int>(info.at(1)));
				//GetActorValue(reference.get(), true, getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func::SetActorValue:
				break;

			case Func::GetActorState:
			{
				auto reference = GameFactory::GetObject<Actor>(getFrom<unsigned int>(info.at(1)));
				GetActorState(reference.get(),
									*reinterpret_cast<unsigned int*>(&result),
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 4),
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 6),
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 5),
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 7));
				break;
			}

			case Func::PlayGroup:
				break;

			case Func::PlayIdle:
				break;

			case Func::Kill:
				break;

			case Func::MoveTo:
				break;

			case Func::Enable:
				break;

			case Func::Disable:
				break;

			case Func::SetRestrained:
				break;

			case Func::SetAlert:
				break;

			case Func::SetForceSneak:
				break;

			case Func::AddItemHealthPercent:
				break;

			case Func::RemoveItem:
				break;

			case Func::EquipItem:
				break;

			case Func::UnequipItem:
				break;

			case Func::FireWeapon:
				break;

			case Func::EnablePlayerControls:
				break;

			case Func::DisablePlayerControls:
				break;

			case Func::SetINISetting:
				break;

			case Func::Lock:
				break;

			case Func::Unlock:
				break;

			case Func::SetOwnership:
				break;

			case Func::Activate:
				break;

			case Func::GUIChat:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				GetMessage(string(reinterpret_cast<char*>(&data[0]), data.size()));
				delete &data;
				break;
			}

			case Func::GUIMode:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				GetWindowMode(data[0]);
				delete &data;
				break;
			}

			case Func::GUICreateWindow:
			case Func::GUICreateButton:
			case Func::GUICreateText:
			case Func::GUICreateEdit:
			case Func::GUIRemoveWindow:
			case Func::GUIPos:
			case Func::GUISize:
			case Func::GUIVisible:
			case Func::GUILocked:
			case Func::GUIMaxLen:
			case Func::GUIValid:
				break;

			case Func::GUIText:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				string name = reinterpret_cast<char*>(&data[0]);
				string text = reinterpret_cast<char*>(&data[0]) + name.length() + 1;
				GetWindowText(move(name), move(text));
				delete &data;
				break;
			}

			case Func::GUIClick:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				GetWindowClick(string(reinterpret_cast<char*>(&data[0]), data.size()));
				delete &data;
				break;
			}

			case Func::GetActivate:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				unsigned int refID = *reinterpret_cast<unsigned int*>(&data[0]);
				auto reference = GameFactory::GetMultiple(vector<unsigned int>{refID, PLAYER_REFERENCE});

				if (reference[0])
					GetActivate(reference[0].get(), reference[1].get());

				delete &data;
				break;
			}

			case Func::SetGlobalValue:
				break;

			case Func::MarkForDelete:
				break;

			case Func::AgeRace:
				break;

			case Func::MatchRace:
				break;

			case Func::SexChange:
				break;

			case Func::ForceWeather:
				break;

			case Func::SetRefCount:
				break;

			case Func::SetCurrentHealth:
				break;

			case Func::UIMessage:
				break;

			case Func::GetParentCell:
			{
				auto player = GameFactory::GetObject<Player>(getFrom<unsigned int>(info.at(1)));
				GetParentCell(player.get(), getFrom<unsigned int>(result));
				break;
			}

			case Func::DisableControl:
				break;

			case Func::GetControl:
			{
				auto self = GameFactory::GetObject<Player>(PLAYER_REFERENCE);
				GetControl(self.get(), getFrom<int>(info.at(1)), result);
				break;
			}

			case Func::DisableKey:
				break;

			case Func::EnableKey:
				break;

			case Func::CenterOnCell:
			case Func::CenterOnWorld:
			case Func::ForceRespawn:
				FutureSet(shared, true);
				break;

			case Func::SetName:
				break;

			default:
				throw VaultException("Unhandled function %04hX", opcode).stacktrace();
		}
	}
	else
	{
#ifdef VAULTMP_DEBUG
		debug.note("Command ", hex, static_cast<unsigned short>(opcode), " failed (");

		for (const auto& value : info)
			debug.note(*reinterpret_cast<const unsigned long long*>(&value), ", ");

		debug.print(")");
#endif

		switch (opcode)
		{
			case Func::PlaceAtMeHealthPercent:
				PlaceAtMe(getFrom<unsigned int>(info.at(1)), getFrom<unsigned int>(info.at(2)), getFrom<unsigned int>(info.at(3)), key);
				break;

			default:
				break;
		}
	}
}

NetworkResponse Game::Authenticate(const string& password)
{
	auto reference = GameFactory::GetObject<Player>(PLAYER_REFERENCE);

	return {Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_AUTH>(reference->GetName(), password),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
	};
}

void Game::Startup()
{
	auto reference = GameFactory::GetObject<Player>(PLAYER_REFERENCE);
	auto& player = reference.get();

	RawParameter self_ref = player->GetReferenceParam();
	NetworkID id = player->GetNetworkID();

	GameFactory::LeaveReference(player);

	SetINISetting("bSaveOnInteriorExteriorSwitch:GamePlay", "0");
	SetINISetting("bSaveOnTravel:GamePlay", "0");
	SetINISetting("bSaveOnWait:GamePlay", "0");
	SetINISetting("bSaveOnRest:GamePlay", "0");
	SetGlobalValue(Global_TimeScale, 0);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GetControl, {RawParameter(API::RetrieveAllControls())});
	Interface::ExecuteCommand(Func::DisableControl, {RawParameter(vector<unsigned char>{
		ControlCode_Quickload,
		ControlCode_Quicksave,
		ControlCode_VATS,
		ControlCode_Rest})});
	Interface::ExecuteCommand(Func::DisableKey, {RawParameter(vector<unsigned int>{
		ScanCode_Escape,
		ScanCode_Console})});

	EnablePlayerControls();

	Interface::EndDynamic();

	Interface::StartSetup();

	Interface::SetupCommand(Func::GetPos, {self_ref, Object::Param_Axis()});
	Interface::SetupCommand(Func::GetPos, {Player::CreateFunctor(FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE), Object::Param_Axis()}, 30);
	//Interface::SetupCommand("GetPos", {Actor::CreateFunctor(FLAG_ENABLED | FLAG_ALIVE), Object::Param_Axis()}, 30);
	Interface::SetupCommand(Func::GetAngle, {self_ref, RawParameter(vector<string> {API::RetrieveAxis_Reverse(Axis_X), API::RetrieveAxis_Reverse(Axis_Z)})});
	Interface::SetupCommand(Func::GetActorState, {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Player::CreateFunctor(FLAG_MOVCONTROLS, id)});
	Interface::SetupCommand(Func::GetParentCell, {Player::CreateFunctor(FLAG_SELF | FLAG_ALIVE)}, 30);

/*
	Interface::SetupCommand(Func::GetActorValue, {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Actor::Param_ActorValues()}, 100);
	Interface::SetupCommand(Func::GetBaseActorValue, {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Actor::Param_ActorValues()}, 200);
*/

	Interface::EndSetup();

	startup = true;

	while (!startupQueue.empty())
	{
		startupQueue.front()();
		startupQueue.pop_front();
	}
}

template <typename T>
void Game::FutureSet(const weak_ptr<Lockable>& data, T&& t)
{
	shared_ptr<Lockable> shared = data.lock();
	Lockable* locked = shared.get();

	if (locked == nullptr)
		throw VaultException("Storage has expired").stacktrace();

	Shared<T>* store = dynamic_cast<Shared<T>*>(locked);

	if (store == nullptr)
		throw VaultException("Storage is corrupted").stacktrace();

	(**store) = forward<T>(t);

	try
	{
		store->set_promise();
	}
	catch (...) {}  // for resolving a weird bug with promise already satisfied, investigate
}

void Game::AsyncDispatch(function<void()>&& func)
{
	thread t(move(func));
	t.detach();
}

void Game::JobDispatch(chrono::milliseconds&& time, function<void()>&& func)
{
	// no move supported
	auto func_ = [time, func]() mutable { Interface::PushJob(chrono::steady_clock::now() + time, move(func)); };

	if (!startup)
		startupQueue.emplace_back(func_);
	else
		func_();
}

void Game::DelayOrExecute(const FactoryObject& reference, function<void(unsigned int)>&& func, unsigned int key)
{
	if (!reference->GetReference())
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	if (!IsInContext(reference->GetGameCell()))
	{
		if (key)
			Lockable::Retrieve(key);
		function<void()> task = bind(func, 0x00000000);
		reference->Enqueue(task);
	}
	else
		func(key);
}

void Game::LoadGame(string savegame)
{
	static string last_savegame;

	if (savegame.empty())
		savegame = last_savegame;
	else
	{
		Utils::RemoveExtension(savegame);
		last_savegame = savegame;
	}

	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::Load, {RawParameter(savegame)}, key);

	Interface::EndDynamic();

	try
	{
		store->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of savegame %s failed (%s)", savegame.c_str(), e.what()).stacktrace();
	}

	// ready state
}

void Game::CenterOnCell(const string& cell, bool spawn)
{
	bool first = false;

	if (spawn)
	{
		first = !spawnFunc.operator bool();
		spawnFunc = bind(CenterOnCell, cell, false);

		if (!first)
			return;
	}

	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::CenterOnCell, {RawParameter(cell)}, key);

	Interface::EndDynamic();

	try
	{
		store->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of cell %s failed (%s)", cell.c_str(), e.what()).stacktrace();
	}

	if (first)
		Game::LoadEnvironment();

	// ready state
}

void Game::CenterOnExterior(signed int x, signed int y, bool spawn)
{
	bool first = false;

	if (spawn)
	{
		first = !spawnFunc.operator bool();
		spawnFunc = bind(CenterOnExterior, x, y, false);

		if (!first)
			return;
	}

	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::CenterOnExterior, {RawParameter(x), RawParameter(y)}, key);

	Interface::EndDynamic();

	try
	{
		store->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of cell (%d,%d) failed (%s)", x, y, e.what()).stacktrace();
	}

	if (first)
		Game::LoadEnvironment();

	// ready state
}

void Game::CenterOnWorld(unsigned int baseID, signed int x, signed int y, bool spawn)
{
	bool first = false;

	if (spawn)
	{
		first = !spawnFunc.operator bool();
		spawnFunc = bind(CenterOnWorld, baseID, x, y, false);

		if (!first)
			return;
	}

	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::CenterOnWorld, {RawParameter(baseID), RawParameter(x), RawParameter(y)}, key);

	Interface::EndDynamic();

	try
	{
		store->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of world (%08X,%d,%d) failed (%s)", baseID, x, y, e.what()).stacktrace();
	}

	if (first)
		Game::LoadEnvironment();

	// ready state
}

void Game::SetINISetting(const string& key, const string& value)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::SetINISetting, {RawParameter(key), RawParameter(value)});

	Interface::EndDynamic();
}

void Game::SetGlobalValue(unsigned int global, signed int value)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::SetGlobalValue, {RawParameter(global), RawParameter(value)});

	Interface::EndDynamic();
}

void Game::LoadEnvironment()
{
	for (const auto& global : globals)
		SetGlobalValue(global.first, global.second);

	if (Game::weather)
		SetWeather(Game::weather);

	vector<NetworkID> reference = GameFactory::GetIDObjectTypes(ALL_OBJECTS);

	uninitObj.StartSession();
	uninitObj->clear();
	uninitObj.EndSession();

	deletedObj.StartSession();

	deletedStatic.StartSession();
	(*deletedObj) = (*deletedStatic);
	deletedStatic.EndSession();

	for (unsigned int cell : spawnContext)
		if (cell)
		{
			for (unsigned int refID : (*deletedObj)[cell])
				RemoveObject(refID);

			(*deletedObj)[cell].clear();
		}

	deletedObj.EndSession();

	for (NetworkID& id : reference)
	{
		GameFactory::Operate(id, [](FactoryObject& object) {
			if (!object->IsPersistent() || object->GetReference() == PLAYER_REFERENCE)
			{
				cellRefs.StartSession();
				(*cellRefs)[object->GetNetworkCell()][object.GetType()].erase(object->GetReference());
				cellRefs.EndSession();

				if (object->GetReference() != PLAYER_REFERENCE)
					object->SetReference(0x00000000);
				else
				{
					object->SetNetworkCell(spawnContext[0]);
					object->SetGameCell(spawnContext[0]);
				}
			}

			NewDispatch(object);
		});
	}
}

void Game::NewDispatch(FactoryObject& reference)
{
	NetworkID id = reference->GetNetworkID();
	unsigned int type = reference.GetType();

	switch (type)
	{
		case ID_OBJECT:
			NewObject(reference);
			break;

		case ID_ITEM:
		{
			auto item = GameFactory::GetObject<Item>(id).get();
			GameFactory::LeaveReference(reference);

			if (!item->GetItemContainer())
				NewItem(item);
			break;
		}

		case ID_CONTAINER:
		{
			auto container = GameFactory::GetObject<Container>(id).get();
			GameFactory::LeaveReference(reference);
			NewContainer(container);
			break;
		}

		case ID_ACTOR:
		{
			auto actor = GameFactory::GetObject<Actor>(id).get();
			GameFactory::LeaveReference(reference);
			NewActor(actor);
			break;
		}

		case ID_PLAYER:
		{
			auto player = GameFactory::GetObject<Player>(id).get();
			GameFactory::LeaveReference(reference);
			NewPlayer(player);
			break;
		}

		default:
			throw VaultException("Can't create object of unknown type %08X", type).stacktrace();
	}
}

void Game::UIMessage(const string& message, unsigned char emoticon)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::UIMessage, {RawParameter(message), RawParameter(emoticon)});

	Interface::EndDynamic();
}

void Game::ChatMessage(const string& message)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIChat, {RawParameter(message)});

	Interface::EndDynamic();
}

void Game::NewObject(FactoryObject& reference)
{
	if (IsInContext(reference->GetNetworkCell()))
		NewObject_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.StartSession();
		(*uninitObj)[reference->GetNetworkCell()].emplace(reference->GetNetworkID());
		uninitObj.EndSession();
	}
}

void Game::NewObject_(FactoryObject& reference)
{
	reference->Release();

	if (!reference->GetReference())
	{
		auto store = make_shared<Shared<unsigned int>>();
		unsigned int key = Lockable::Share(store);

		auto item = vaultcast<Item>(reference);
		double condition = item ? (item->GetItemCondition() / 100.0) : 1.00;

		unsigned int baseID = reference->GetBase();
		PlaceAtMe(PLAYER_REFERENCE, baseID, condition, 1, key);

		NetworkID id = reference->GetNetworkID();

		GameFactory::LeaveReference(reference);

		unsigned int refID;

		try
		{
			refID = store->get_future(chrono::seconds(15));
		}
		catch (exception& e)
		{
			throw VaultException("Object creation with baseID %08X and NetworkID %llu failed (%s)", baseID, id, e.what()).stacktrace();
		}

		reference = GameFactory::GetObject(id).get();
		reference->SetReference(refID);
	}

	reference->SetEnabled(true);

	SetName(reference);
	SetAngle(reference);

	//if (reference->GetLockLevel() != Lock_Unlocked)
		SetLock(reference);

	if (reference->GetOwner())
		SetOwner(reference);

	unsigned int refID = reference->GetReference();

	// experimental
	if (refID != PLAYER_REFERENCE)
	{
		if (!reference->IsPersistent())
		{
			JobDispatch(chrono::milliseconds(500), [refID]
			{
				try
				{
					auto objects = GameFactory::GetMultiple(vector<unsigned int>{refID, PLAYER_REFERENCE});

					auto& object = objects[0].get();
					auto& player = objects[1].get();

					MoveTo(object, player, true);

					object->SetGameCell(player->GetGameCell());
					object->Work();
				}
				catch (...) {}
			});
		}
	}

	cellRefs.StartSession();
	(*cellRefs)[reference->GetNetworkCell()][reference.GetType()].insert(refID);
	cellRefs.EndSession();

	// maybe more
}

void Game::NewItem(FactoryItem& reference)
{
	NetworkID item = reference->GetNetworkID();
	NetworkID container = reference->GetItemContainer();

	if (container)
	{
		GameFactory::LeaveReference(reference);

		GameFactory::Operate<Container>(container, [item](FactoryContainer& container) {
			container->IL.AddItem(item);

			GameFactory::Operate<Item>(item, [&container](FactoryItem& item) {
				AddItem(container, item);

				if (item->GetItemEquipped())
					GameFactory::Operate<Actor>(container->GetNetworkID(), [&item](FactoryActor& actor) {
						EquipItem(actor, item);
					});
			});
		});
	}
	else
	{
		if (IsInContext(reference->GetNetworkCell()))
			NewItem_(reference);
		else
		{
			reference->SetEnabled(false);

			uninitObj.StartSession();
			(*uninitObj)[reference->GetNetworkCell()].emplace(reference->GetNetworkID());
			uninitObj.EndSession();
		}
	}
}

void Game::NewItem_(FactoryItem& reference)
{
	NetworkID id = reference->GetItemContainer();

	if (id)
		throw VaultException("Cannot create item %llu which is bound to a Container (%llu)", reference->GetNetworkID(), id).stacktrace();

	NewObject_(reference);
	SetRefCount(reference);
	// add SetCurrentHealth
}

void Game::NewContainer(FactoryContainer& reference)
{
	if (IsInContext(reference->GetNetworkCell()))
		NewContainer_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.StartSession();
		(*uninitObj)[reference->GetNetworkCell()].emplace(reference->GetNetworkID());
		uninitObj.EndSession();
	}
}

void Game::NewContainer_(FactoryContainer& reference)
{
	NewObject_(reference);
	auto items = GameFactory::GetMultiple<Item>(vector<NetworkID>(reference->IL.GetItemList().begin(), reference->IL.GetItemList().end()));

	for (auto& _item : items)
	{
		AddItem(reference, _item.get());
		//debug->PrintFormat("ID: %llu, %s, %08X, %d, %d, %d, %d", true, item->GetNetworkID(), item->GetName().c_str(), item->GetBase(), (int)item->GetItemEquipped(), (int)item->GetItemSilent(), (int)item->GetItemStick(), item->GetItemCount());

		if (_item->GetItemEquipped())
			EquipItem(vaultcast<Actor>(reference).get(), _item.get());
	}
}

void Game::NewActor(FactoryActor& reference)
{
	if (IsInContext(reference->GetNetworkCell()))
		NewActor_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.StartSession();
		(*uninitObj)[reference->GetNetworkCell()].emplace(reference->GetNetworkID());
		uninitObj.EndSession();
	}
}

void Game::NewActor_(FactoryActor& reference)
{
	NewContainer_(reference);

	vector<unsigned char> values = API::RetrieveAllValues();

	for (unsigned char value : values)
	{
		SetActorValue(reference, true, value);
		SetActorValue(reference, false, value);
	}

	SetActorRace(reference, reference->GetActorAge()); // FIXME - AgeRace is a delta, do only once per base (redo on respawn). sucks a bit
	SetActorFemale(reference);

	if (reference->GetReference() != PLAYER_REFERENCE)
	{
		SetRestrained(reference, true);

		if (!reference->GetActorDead())
		{
			if (reference->GetActorAlerted())
				SetActorAlerted(reference);

			if (reference->GetActorSneaking())
				SetActorSneaking(reference);

			if (reference->GetActorMovingAnimation() != AnimGroup_Idle)
				SetActorMovingAnimation(reference);

			if (reference->GetActorWeaponAnimation() != AnimGroup_Idle)
				SetActorWeaponAnimation(reference);
		}
		else
			KillActor(reference, 0, Death_None);
	}
}

void Game::NewPlayer(FactoryPlayer& reference)
{
	if (IsInContext(reference->GetNetworkCell()) || reference->GetReference() == PLAYER_REFERENCE)
		NewPlayer_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.StartSession();
		(*uninitObj)[reference->GetNetworkCell()].emplace(reference->GetNetworkID());
		uninitObj.EndSession();
	}
}

void Game::NewPlayer_(FactoryPlayer& reference)
{
	NewActor_(reference);

	// ...
}

void Game::RemoveObject(const FactoryObject& reference)
{
	if (!reference->GetReference())
		return;

	if (IsInContext(reference->GetGameCell()))
	{
		if (reference->SetEnabled(false))
			ToggleEnabled(reference);

		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::MarkForDelete, {reference->GetReferenceParam()});

		Interface::EndDynamic();
	}
	else
	{
		deletedObj.StartSession();
		(*deletedObj)[reference->GetGameCell()].emplace_back(reference->GetReference());
		deletedObj.EndSession();
	}

	cellRefs.StartSession();
	(*cellRefs)[reference->GetNetworkCell()][reference.GetType()].erase(reference->GetReference());
	cellRefs.EndSession();
}

void Game::RemoveObject(unsigned int refID)
{
	ToggleEnabled(refID, false);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::MarkForDelete, {RawParameter(refID)});

	Interface::EndDynamic();
}

void Game::NewWindow(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	if (reference->GetLabel().empty())
	{
		reference->SetLabel(Utils::toString(reference->GetNetworkID()));

		Interface::ExecuteCommand(Func::GUICreateWindow, {RawParameter(reference->GetLabel())});
	}

	SetWindowPos(reference);
	SetWindowSize(reference);
	SetWindowVisible(reference);
	SetWindowLocked(reference);
	SetWindowText(reference);

	Interface::EndDynamic();
}

void Game::NewButton(const FactoryButton& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::StartDynamic();

	if (reference->GetLabel().empty())
	{
		reference->SetLabel(Utils::toString(reference->GetNetworkID()));

		Interface::ExecuteCommand(Func::GUICreateButton, {RawParameter(Utils::toString(reference->GetParentWindow())), RawParameter(reference->GetLabel())});
	}

	NewWindow(reference);

	Interface::EndDynamic();
}

void Game::NewText(const FactoryText& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::StartDynamic();

	if (reference->GetLabel().empty())
	{
		reference->SetLabel(Utils::toString(reference->GetNetworkID()));

		Interface::ExecuteCommand(Func::GUICreateText, {RawParameter(Utils::toString(reference->GetParentWindow())), RawParameter(reference->GetLabel())});
	}

	NewWindow(reference);

	Interface::EndDynamic();
}

void Game::NewEdit(const FactoryEdit& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::StartDynamic();

	if (reference->GetLabel().empty())
	{
		reference->SetLabel(Utils::toString(reference->GetNetworkID()));

		Interface::ExecuteCommand(Func::GUICreateEdit, {RawParameter(Utils::toString(reference->GetParentWindow())), RawParameter(reference->GetLabel())});
	}

	NewWindow(reference);

	SetEditMaxLength(reference);
	SetEditValidation(reference);

	Interface::EndDynamic();
}

void Game::PlaceAtMe(const FactoryObject& reference, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	PlaceAtMe(reference->GetReference(), baseID, condition, count, key);
}

void Game::PlaceAtMe(unsigned int refID, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::PlaceAtMeHealthPercent, {RawParameter(refID), RawParameter(baseID), RawParameter(condition), RawParameter(count)}, key);

	Interface::EndDynamic();
}

void Game::ToggleEnabled(const FactoryObject& reference)
{
	ToggleEnabled(reference->GetReference(), reference->GetEnabled());
}

void Game::ToggleEnabled(unsigned int refID, bool enabled)
{
	Interface::StartDynamic();

	if (enabled)
		Interface::ExecuteCommand(Func::Enable, {RawParameter(refID), RawParameter(true)});
	else
		Interface::ExecuteCommand(Func::Disable, {RawParameter(refID), RawParameter(false)});

	Interface::EndDynamic();
}

void Game::DestroyObject(FactoryObject& reference, bool silent)
{
	NetworkID id = reference->GetNetworkID();

	NetworkID container = GameFactory::Operate<Item, FailPolicy::Return>(id, [](FactoryItem& item) {
		return item->GetItemContainer();
	});

	if (container)
	{
		GameFactory::LeaveReference(reference);

		GameFactory::Operate<Container>(container, [id, silent](FactoryContainer& container) {
			container->IL.RemoveItem(id);

			GameFactory::Operate<Item>(id, [&container, silent](FactoryItem& item) {
				RemoveItem(container, item->GetBase(), item->GetItemCount(), silent);

				// Game always removes equipped item first - workaround (is this really always the case?)
				NetworkID equipped = container->IL.IsEquipped(item->GetBase());

				if (equipped)
					GameFactory::Operate<Actor>(container->GetNetworkID(), [equipped](FactoryActor& actor) {
						GameFactory::Operate<Item>(equipped, [&actor](FactoryItem& item) {
							EquipItem(actor, item->GetBase(), item->GetItemCondition(), false, item->GetItemStick());
						});
					});
			});

			GameFactory::DestroyInstance(id);
		});
	}
	else
	{
		RemoveObject(reference);

		uninitObj.StartSession();
		(*uninitObj)[reference->GetNetworkCell()].erase(reference->GetNetworkID());
		uninitObj.EndSession();

		if (reference->IsPersistent())
		{
			deletedStatic.StartSession();
			(*deletedStatic)[reference->GetNetworkCell()].emplace_back(reference->GetReference());
			deletedStatic.EndSession();
		}

		GameFactory::DestroyInstance(reference);
	}
}

void Game::DeleteWindow(FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIRemoveWindow, {RawParameter(reference->GetLabel())});

	Interface::EndDynamic();

	GameFactory::DestroyInstance(reference);
}

unsigned int Game::GetBase(unsigned int refID)
{
	auto store = make_shared<Shared<unsigned int>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GetBaseObject, {RawParameter(refID)}, key);

	Interface::EndDynamic();

	unsigned int baseID;

	try
	{
		baseID = store->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Obtaining of baseID of refID %08X (%s)", refID, e.what()).stacktrace();
	}

	return baseID;
}

void Game::SetName(const FactoryObject& reference)
{
	const string& name = reference->GetName();

	auto* object = reference.operator->();

	auto func = [object, name](unsigned int)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::SetName, {object->GetReferenceParam(), RawParameter(name)});

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, 0);
}

void Game::SetRestrained(const FactoryActor& reference, bool restrained)
{
	//bool restrained = actor->GetActorRestrained();

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::SetRestrained, {reference->GetReferenceParam(), RawParameter(restrained)});

	Interface::EndDynamic();
}

void Game::Activate(const FactoryObject& reference, const FactoryObject& actor)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::Activate, {reference->GetReferenceParam(), actor->GetReferenceParam()});

	Interface::EndDynamic();
}

void Game::SetPos(const FactoryObject& reference)
{
	if (!reference->HasValidCoordinates())
		return;

	Lockable* key = nullptr;

	Interface::StartDynamic();

	key = reference->SetGamePos(Axis_X, reference->GetNetworkPos(Axis_X));

	Interface::ExecuteCommand(Func::SetPos, {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(reference->GetNetworkPos(Axis_X))}, key ? key->Lock() : 0);

	key = reference->SetGamePos(Axis_Y, reference->GetNetworkPos(Axis_Y));

	Interface::ExecuteCommand(Func::SetPos, {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Y)), RawParameter(reference->GetNetworkPos(Axis_Y))}, key ? key->Lock() : 0);

	key = reference->SetGamePos(Axis_Z, reference->GetNetworkPos(Axis_Z));

	Interface::ExecuteCommand(Func::SetPos, {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(reference->GetNetworkPos(Axis_Z))}, key ? key->Lock() : 0);

	Interface::EndDynamic();
}

void Game::SetAngle(const FactoryObject& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::SetAngle, {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(reference->GetAngle(Axis_X))});

	double value = reference->GetAngle(Axis_Z);
	auto actor = vaultcast<Actor>(reference);

	if (actor)
	{
		if (actor->GetActorMovingXY() == 0x01)
			AdjustZAngle(value, -45.0);
		else if (actor->GetActorMovingXY() == 0x02)
			AdjustZAngle(value, 45.0);
	}

	Interface::ExecuteCommand(Func::SetAngle, {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(value)});

	Interface::EndDynamic();
}

void Game::MoveTo(const FactoryObject& reference, const FactoryObject& object, bool cell, unsigned int key)
{
	Interface::StartDynamic();

	ParamContainer param_MoveTo{reference->GetReferenceParam(), object->GetReferenceParam()};

	if (cell)
	{
		param_MoveTo.emplace_back(reference->GetNetworkPos(Axis_X) - object->GetNetworkPos(Axis_X));
		param_MoveTo.emplace_back(reference->GetNetworkPos(Axis_Y) - object->GetNetworkPos(Axis_Y));
		param_MoveTo.emplace_back(reference->GetNetworkPos(Axis_Z) - object->GetNetworkPos(Axis_Z));
	}

	Interface::ExecuteCommand(Func::MoveTo, move(param_MoveTo), key);

	Interface::EndDynamic();
}

void Game::SetLock(const FactoryObject& reference, unsigned int key)
{
	unsigned int lock = reference->GetLockLevel();

	if (lock == Lock_Broken) // workaround: can't set lock to broken, so set it to impossible
		lock = Lock_Impossible;

	auto* object = reference.operator->();

	auto func = [object, lock](unsigned int key)
	{
		Interface::StartDynamic();

		if (lock != Lock_Unlocked)
			Interface::ExecuteCommand(Func::Lock, {object->GetReferenceParam(), RawParameter(lock)}, key);
		else
			Interface::ExecuteCommand(Func::Unlock, {object->GetReferenceParam()}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetOwner(const FactoryObject& reference, unsigned int key)
{
	unsigned int owner = reference->GetOwner();

	if (owner == playerBase)
		owner = PLAYER_BASE;

	auto* object = reference.operator->();

	auto func = [object, owner](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::SetOwnership, {object->GetReferenceParam(), RawParameter(owner)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetActorValue(const FactoryActor& reference, bool base, unsigned char index, unsigned int key)
{
	auto* actor = reference.operator->();

	double value = base ? actor->GetActorBaseValue(index) : actor->GetActorValue(index);

	auto func = [actor, base, index, value](unsigned int key)
	{
		Interface::StartDynamic();

		if (base)
			Interface::ExecuteCommand(Func::SetActorValue, {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);
		else
			Interface::ExecuteCommand(Func::ForceActorValue, {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::DamageActorValue(const FactoryActor& reference, unsigned char index, double value, unsigned int key)
{
	auto* actor = reference.operator->();

	auto func = [actor, index, value](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::DamageActorValue, {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::RestoreActorValue(const FactoryActor& reference, unsigned char index, double value, unsigned int key)
{
	auto* actor = reference.operator->();

	auto func = [actor, index, value](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::RestoreActorValue, {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetActorSneaking(const FactoryActor& reference, unsigned int key)
{
	auto* actor = reference.operator->();

	bool sneaking = actor->GetActorSneaking();

	auto func = [actor, sneaking](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::SetAlert, {actor->GetReferenceParam(), RawParameter(sneaking)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetActorAlerted(const FactoryActor& reference, unsigned int key)
{
	auto* actor = reference.operator->();

	bool alerted = actor->GetActorAlerted();

	auto func = [actor, alerted](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::SetAlert, {actor->GetReferenceParam(), RawParameter(alerted)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetActorAnimation(const FactoryActor& reference, unsigned char anim, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::PlayGroup, {reference->GetReferenceParam(), RawParameter(API::RetrieveAnim_Reverse(anim)), RawParameter(true)}, key);

	Interface::EndDynamic();
}

void Game::SetActorMovingAnimation(const FactoryActor& reference, unsigned int key)
{
	SetActorAnimation(reference, reference->GetActorMovingAnimation(), key);
}

void Game::SetActorWeaponAnimation(const FactoryActor& reference, unsigned int key)
{
	SetActorAnimation(reference, reference->GetActorWeaponAnimation(), key);
}

void Game::SetActorIdleAnimation(const FactoryActor& reference, const string& anim, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::PlayIdle, {reference->GetReferenceParam(), RawParameter(anim)}, key);

	Interface::EndDynamic();
}

void Game::SetActorRace(const FactoryActor& reference, signed int delta_age, unsigned int key)
{
	auto* actor = reference.operator->();

	unsigned int race = actor->GetActorRace();

	auto func = [actor, race, delta_age](unsigned int key)
	{
		unsigned int baseID = actor->GetBase();

		// set only once per base
		if (baseRaces[baseID] == race || race == UINT_MAX) // creature test
		{
			if (key)
				Lockable::Retrieve(key);
			return;
		}

		baseRaces[baseID] = race;

		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::MatchRace, {actor->GetReferenceParam(), RawParameter(race)}, delta_age ? 0 : key);

		if (delta_age)
			Interface::ExecuteCommand(Func::AgeRace, {actor->GetReferenceParam(), RawParameter(delta_age)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetActorFemale(const FactoryActor& reference, unsigned int key)
{
	auto* actor = reference.operator->();

	bool female = actor->GetActorFemale();

	auto func = [actor, female](unsigned int key)
	{
		if (actor->GetActorRace() == UINT_MAX) // creature test
		{
			if (key)
				Lockable::Retrieve(key);
			return;
		}

		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::SexChange, {actor->GetReferenceParam(), RawParameter(female)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::KillActor(const FactoryActor& reference, unsigned short limbs, signed char cause, unsigned int key)
{
	auto* actor = reference.operator->();

	auto func = [actor, limbs, cause](unsigned int key)
	{
		Interface::StartDynamic();

		// maybe add valid killer later
		if (limbs)
		{
			unsigned int j = 0;

			for (unsigned int i = 1; i <= limbs; i <<= 1, ++j)
				if (limbs & i)
					Interface::ExecuteCommand(Func::Kill, {actor->GetReferenceParam(), actor->GetReferenceParam(), RawParameter(j), RawParameter(cause)}, ((i << 1) > limbs) ? key : 0x00);
		}
		else
			Interface::ExecuteCommand(Func::Kill, {actor->GetReferenceParam(), actor->GetReferenceParam(), RawParameter(Limb_None), RawParameter(cause)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::FireWeapon(const FactoryActor& reference, unsigned int weapon, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::FireWeapon, {reference->GetReferenceParam(), RawParameter(weapon)}, key);

	Interface::EndDynamic();
}

void Game::AddItem(const FactoryContainer& reference, const FactoryItem& item, unsigned int key)
{
	AddItem(reference, item->GetBase(), item->GetItemCount(), item->GetItemCondition(), item->GetItemSilent(), key);
}

void Game::AddItem(const FactoryContainer& reference, unsigned int baseID, unsigned int count, double condition, bool silent, unsigned int key)
{
	auto* container = reference.operator->();

	auto func = [container, baseID, count, condition, silent](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::AddItemHealthPercent, {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(condition / 100), RawParameter(silent)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::RemoveItem(const FactoryContainer& reference, const FactoryItem& item, unsigned int key)
{
	RemoveItem(reference, item->GetBase(), item->GetItemCount(), item->GetItemSilent(), key);
}

void Game::RemoveItem(const FactoryContainer& reference, unsigned int baseID, unsigned int count, bool silent, unsigned int key)
{
	auto* container = reference.operator->();

	auto func = [container, baseID, count, silent](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::RemoveItem, {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(silent)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetRefCount(const FactoryItem& reference, unsigned int key)
{
	auto* item = reference.operator->();

	unsigned int count = item->GetItemCount();

	auto func = [item, count](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::SetRefCount, {item->GetReferenceParam(), RawParameter(count)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetCurrentHealth(const FactoryItem& reference, unsigned int health, unsigned int key)
{
	auto* item = reference.operator->();

	auto func = [item, health](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::SetCurrentHealth, {item->GetReferenceParam(), RawParameter(health)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::EquipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key)
{
	EquipItem(reference, item->GetBase(), item->GetItemCondition(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::EquipItem(const FactoryActor& reference, unsigned int baseID, double condition, bool silent, bool stick, unsigned int key)
{
	auto* actor = reference.operator->();

	auto func = [actor, baseID, condition, silent, stick](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::EquipItem, {actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

		Interface::EndDynamic();

		// Add: adjust condition
	};

	DelayOrExecute(reference, func, key);
}

void Game::UnequipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key)
{
	UnequipItem(reference, item->GetBase(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::UnequipItem(const FactoryActor& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	auto* actor = reference.operator->();

	auto func = [actor, baseID, silent, stick](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand(Func::UnequipItem, {actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetWindowPos(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIPos, {RawParameter(reference->GetLabel()), RawParameter(get<0>(reference->GetPos())), RawParameter(get<1>(reference->GetPos())), RawParameter(get<2>(reference->GetPos())), RawParameter(get<3>(reference->GetPos()))});

	Interface::EndDynamic();
}

void Game::SetWindowSize(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUISize, {RawParameter(reference->GetLabel()), RawParameter(get<0>(reference->GetSize())), RawParameter(get<1>(reference->GetSize())), RawParameter(get<2>(reference->GetSize())), RawParameter(get<3>(reference->GetSize()))});

	Interface::EndDynamic();
}

void Game::SetWindowVisible(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIVisible, {RawParameter(reference->GetLabel()), RawParameter(reference->GetVisible())});

	Interface::EndDynamic();
}

void Game::SetWindowLocked(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUILocked, {RawParameter(reference->GetLabel()), RawParameter(reference->GetLocked())});

	Interface::EndDynamic();
}

void Game::SetWindowText(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIText, {RawParameter(reference->GetLabel()), RawParameter(reference->GetText())});

	Interface::EndDynamic();
}

void Game::SetEditMaxLength(const FactoryEdit& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIMaxLen, {RawParameter(reference->GetLabel()), RawParameter(reference->GetMaxLength())});

	Interface::EndDynamic();
}

void Game::SetEditValidation(const FactoryEdit& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIValid, {RawParameter(reference->GetLabel()), RawParameter(reference->GetValidation())});

	Interface::EndDynamic();
}

void Game::SetWindowMode()
{
	if (GUIMode)
		DisablePlayerControls(true, true, true, false);
	else
		EnablePlayerControls();

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::GUIMode, {RawParameter(GUIMode)});

	Interface::EndDynamic();
}

void Game::EnablePlayerControls(bool movement, bool pipboy, bool fighting, bool pov, bool looking, bool rollover, bool sneaking)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::EnablePlayerControls, {RawParameter(movement), RawParameter(pipboy), RawParameter(fighting), RawParameter(pov), RawParameter(looking), RawParameter(rollover), RawParameter(sneaking)});

	Interface::EndDynamic();
}

void Game::DisablePlayerControls(bool movement, bool pipboy, bool fighting, bool pov, bool looking, bool rollover, bool sneaking)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::DisablePlayerControls, {RawParameter(movement), RawParameter(pipboy), RawParameter(fighting), RawParameter(pov), RawParameter(looking), RawParameter(rollover), RawParameter(sneaking)});

	Interface::EndDynamic();
}

void Game::SetWeather(unsigned int weather)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::ForceWeather, {RawParameter(weather), RawParameter(1)});

	Interface::EndDynamic();
}

void Game::ForceRespawn()
{
	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand(Func::ForceRespawn, {}, key);

	Interface::EndDynamic();

	try
	{
		store->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Respawning failed (%s)", e.what()).stacktrace();
	}
}

bool Game::IsInContext(unsigned int cell)
{
	if (!cell)
		return false;

	bool result;

	cellContext.StartSession();
	result = find(cellContext->begin(), cellContext->end(), cell) != cellContext->end();
	cellContext.EndSession();

	return result;
}

vector<unsigned int> Game::GetContext(unsigned int type)
{
	vector<unsigned int> result;

	cellRefs.StartSession();
	cellContext.StartSession();

	for (unsigned int cell : *cellContext)
		if (cell)
			for (const auto& refs : (*cellRefs)[cell])
				if (refs.first & type)
					result.insert(result.end(), refs.second.begin(), refs.second.end());

	cellContext.EndSession();
	cellRefs.EndSession();

	return result;
}

void Game::net_SetName(const FactoryObject& reference, const string& name)
{
	bool result = static_cast<bool>(reference->SetName(name));

	if (result)
		SetName(reference);
}

void Game::net_SetPos(const FactoryObject& reference, double X, double Y, double Z)
{
	bool result = (static_cast<bool>(reference->SetNetworkPos(Axis_X, X)) | static_cast<bool>(reference->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(reference->SetNetworkPos(Axis_Z, Z)));

	if (result && reference->GetEnabled())
	{
		auto actor = vaultcast<Actor>(reference); // maybe we should consider items, too (they have physics)

		if (!actor || (!reference->IsNearPoint(reference->GetNetworkPos(Axis_X), reference->GetNetworkPos(Axis_Y), reference->GetNetworkPos(Axis_Z), 50.0)) || actor->IsActorJumping() || actor->GetReference() == PLAYER_REFERENCE)
			SetPos(reference);
	}
}

void Game::net_SetAngle(const FactoryObject& reference, unsigned char axis, double value)
{
	bool result = static_cast<bool>(reference->SetAngle(axis, value));

	if (result && reference->GetEnabled())
	{
		SetAngle(reference);

		if (axis == Axis_X)
		{
			auto actor = vaultcast<Actor>(reference);

			if (actor && actor->GetActorWeaponAnimation() == AnimGroup_AimIS)
			{
				SetActorAnimation(actor.get(), AnimGroup_AimISDown);
				SetActorAnimation(actor.get(), AnimGroup_AimISUp);
			}
		}
	}
}

void Game::net_SetCell(FactoryObject& reference, FactoryPlayer& player, unsigned int cell, double X, double Y, double Z)
{
	unsigned int old_cell = reference->GetNetworkCell();
	reference->SetNetworkCell(cell);

	bool result = false;

	if (X || Y || Z)
		result = (static_cast<bool>(reference->SetNetworkPos(Axis_X, X)) | static_cast<bool>(reference->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(reference->SetNetworkPos(Axis_Z, Z)));

	if (reference->GetReference())
	{
		if (reference != player)
		{
			if (IsInContext(cell))
			{
				if (vaultcast<Actor>(reference) || IsInContext(old_cell))
				{
					cellRefs.StartSession();
					(*cellRefs)[old_cell][reference.GetType()].erase(reference->GetReference());
					(*cellRefs)[cell][reference.GetType()].insert(reference->GetReference());
					cellRefs.EndSession();

					if (reference->SetEnabled(true))
						ToggleEnabled(reference);

					if (reference->SetGameCell(cell))
						MoveTo(reference, player, true);
				}
				else
				{
					cellRefs.StartSession();
					(*cellRefs)[old_cell][reference.GetType()].erase(reference->GetReference());
					cellRefs.EndSession();

					RemoveObject(reference);
					reference->SetReference(0x00000000);
					reference->SetEnabled(false);

					GameFactory::LeaveReference(player);
					NewDispatch(reference);
				}
			}
			else
			{
				if (vaultcast<Actor>(reference))
				{
					cellRefs.StartSession();
					(*cellRefs)[old_cell][reference.GetType()].erase(reference->GetReference());
					(*cellRefs)[cell][reference.GetType()].insert(reference->GetReference());
					cellRefs.EndSession();

					if (reference->SetEnabled(false))
						ToggleEnabled(reference);
				}
				else
				{
					cellRefs.StartSession();
					(*cellRefs)[old_cell][reference.GetType()].erase(reference->GetReference());
					cellRefs.EndSession();

					uninitObj.StartSession();
					(*uninitObj)[cell].emplace(reference->GetNetworkID());
					uninitObj.EndSession();

					RemoveObject(reference);
					reference->SetReference(0x00000000);
					reference->SetEnabled(false);
				}
			}
		}
		else if (result)
			SetPos(player);
	}
	else
	{
		bool context = IsInContext(cell);

		uninitObj.StartSession();
		(*uninitObj)[old_cell].erase(reference->GetNetworkID());
		if (!context)
			(*uninitObj)[cell].insert(reference->GetNetworkID());
		uninitObj.EndSession();

		if (context)
		{
			GameFactory::LeaveReference(player);
			NewDispatch(reference);
		}
	}
}

void Game::net_SetLock(const FactoryObject& reference, unsigned int lock)
{
	Lockable* result;

	if ((result = reference->SetLockLevel(lock)))
		SetLock(reference);
}

void Game::net_SetOwner(const FactoryObject& reference, unsigned int owner)
{
	if (reference->SetOwner(owner))
		SetOwner(reference);
}

void Game::net_GetActivate(const FactoryObject& reference, const FactoryObject& actor)
{
	Activate(reference, actor);
}

void Game::net_SetItemCount(FactoryItem& reference, unsigned int count, bool silent)
{
	unsigned int old_count = reference->GetItemCount();

	if (reference->SetItemCount(count))
	{
		NetworkID container = reference->GetItemContainer();

		if (!container)
			SetRefCount(reference);
		else
		{
			reference->SetItemSilent(silent);

			unsigned int baseID = reference->GetBase();
			double condition = reference->GetItemCondition();

			GameFactory::LeaveReference(reference);

			GameFactory::Operate<Container>(container, [count, old_count, baseID, condition, silent](FactoryContainer& container) {
				signed int diff = count - old_count;

				if (diff > 0)
					AddItem(container, baseID, diff, condition, silent);
				else
				{
					RemoveItem(container, baseID, abs(diff), silent);

					// Game always removes equipped item first - workaround (is this really always the case?)
					NetworkID equipped = container->IL.IsEquipped(baseID);

					if (equipped)
						GameFactory::Operate<Actor>(container->GetNetworkID(), [equipped](FactoryActor& actor) {
							GameFactory::Operate<Item>(equipped, [&actor](FactoryItem& item) {
								EquipItem(actor, item->GetBase(), item->GetItemCondition(), false, item->GetItemStick());
							});
						});
				}
			});
		}
	}
}

void Game::net_SetItemCondition(FactoryItem& reference, double condition, unsigned int health)
{
	if (reference->SetItemCondition(condition))
	{
		NetworkID container = reference->GetItemContainer();

		if (!container)
			SetCurrentHealth(reference, health);
		else if (reference->GetItemEquipped())
		{
			GameFactory::LeaveReference(reference);

			// SetEquippedCurrentHealth
		}
	}

}

void Game::net_SetItemEquipped(FactoryItem& reference, bool equipped, bool silent, bool stick)
{
	if (reference->SetItemEquipped(equipped))
	{
		reference->SetItemSilent(silent);
		reference->SetItemStick(stick);

		NetworkID item = reference->GetNetworkID();
		NetworkID container = reference->GetItemContainer();

		GameFactory::LeaveReference(reference);

		GameFactory::Operate<Actor>(container, [equipped, item](FactoryActor& actor) {
			GameFactory::Operate<Item>(item, [&actor, equipped](FactoryItem& item) {
				if (equipped)
					EquipItem(actor, item);
				else
					UnequipItem(actor, item);
			});
		});
	}
}

void Game::net_SetActorValue(const FactoryActor& reference, bool base, unsigned char index, double value)
{
	Lockable* result;

	double prev_value = reference->GetActorValue(index);

	if (base)
		result = reference->SetActorBaseValue(index, value);
	else
		result = reference->SetActorValue(index, value);

	if (result)
	{
		if (!base && (index == ActorVal_Health || (index >= ActorVal_Head && index <= ActorVal_Brain)))
		{
			double diff = value - prev_value;

			if (diff < 0.00)
				DamageActorValue(reference, index, diff);
			else if (diff > 0.00)
				RestoreActorValue(reference, index, diff);
		}
		else
			SetActorValue(reference, base, index);
	}
}

void Game::net_SetActorState(const FactoryActor& reference, unsigned int, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool firing)
{
	Lockable* result;
	bool enabled = reference->GetEnabled();

	result = reference->SetActorMovingXY(movingxy);

	if (result && enabled)
		SetAngle(reference);

	result = reference->SetActorAlerted(alerted);

	if (result && enabled)
		SetActorAlerted(reference, result->Lock());

	result = reference->SetActorSneaking(sneaking);

	if (result && enabled)
		SetActorSneaking(reference, result->Lock());

	result = reference->SetActorMovingAnimation(moving);

	if (result && enabled)
	{
		SetActorMovingAnimation(reference, result->Lock());

		if (moving == AnimGroup_Idle)
			SetPos(reference);
	}

	unsigned char prev_weapon = reference->GetActorWeaponAnimation();
	result = reference->SetActorWeaponAnimation(weapon);

	if (result && enabled && !firing && reference->GetActorAlerted() && weapon != AnimGroup_Idle && weapon != AnimGroup_Equip && weapon != AnimGroup_Unequip && weapon != AnimGroup_Holster &&
		//(game == FALLOUT3 ? (weapon == AnimGroup_BlockHit) : (weapon == FalloutNV::AnimGroup_BlockHit)) &&
		(weapon != AnimGroup_Aim || prev_weapon == AnimGroup_AimIS))
	{
		if (weapon == AnimGroup_Aim && prev_weapon == AnimGroup_AimIS)
		{
			SetActorAnimation(reference, AnimGroup_AimDown);
			SetActorAnimation(reference, AnimGroup_AimUp);
		}

		SetActorWeaponAnimation(reference, result->Lock());

		if (weapon == AnimGroup_AimIS)
		{
			SetActorAnimation(reference, AnimGroup_AimISDown);
			SetActorAnimation(reference, AnimGroup_AimISUp);
		}
	}
}

void Game::net_SetActorRace(const FactoryActor& reference, unsigned int race, signed int age, signed int delta_age)
{
	reference->SetActorRace(race);
	reference->SetActorAge(age); // delta from original race to new race
	SetActorRace(reference, delta_age); // using delta from current race to new race
}

void Game::net_SetActorFemale(const FactoryActor& reference, bool female)
{
	reference->SetActorFemale(female);
	SetActorFemale(reference);
}

void Game::net_SetActorDead(FactoryActor& reference, bool dead, unsigned short limbs, signed char cause)
{
	Lockable* result;

	result = reference->SetActorDead(dead);

	if (result && reference->GetReference())
	{
		if (dead)
			KillActor(reference, limbs, cause);
		else if (reference->GetReference() != PLAYER_REFERENCE)
		{
			RemoveObject(reference);
			reference->SetReference(0x00000000);
			NewActor(reference);
		}
		else
		{
			NetworkID id = reference->GetNetworkID();
			reference->SetEnabled(false);
			GameFactory::LeaveReference(reference);

			Game::ForceRespawn();

			this_thread::sleep_for(chrono::seconds(1));

			// remove all base effects so they get re-applied in LoadEnvironment
			Game::baseRaces.clear();
			Game::spawnFunc();

			cellContext.StartSession();
			(*cellContext) = spawnContext;
			cellContext.EndSession();

			Game::LoadEnvironment();

			reference = GameFactory::GetObject<Actor>(id).get();
			reference->SetEnabled(true);
			GameFactory::LeaveReference(reference);

			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, false, 0, 0),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::net_FireWeapon(const FactoryActor& reference, unsigned int weapon, double rate)
{
	bool enabled = reference->GetEnabled();

	if (enabled)
	{
		FireWeapon(reference, weapon);
		NetworkID id = reference->GetNetworkID();

		if (rate)
		{
			// automatic weapons

			AsyncDispatch([id, weapon, rate]
			{
				try
				{
					ExpectedActor reference;

					// rate: per second
					auto us = chrono::microseconds(static_cast<unsigned long long>(1000000ull / rate));

					this_thread::sleep_for(us);

					while ((reference = GameFactory::GetObject<Actor>(id)) && reference->IsActorFiring() && reference->IL.IsEquipped(weapon))
					{
						auto& _reference = reference.get();
						FireWeapon(_reference, weapon);
						GameFactory::LeaveReference(_reference);
						this_thread::sleep_for(us);
					}
				}
				catch (...) {}
			});
		}
	}
}

void Game::net_SetActorIdle(const FactoryActor& reference, unsigned int idle, const string& name)
{
	Lockable* result;
	bool enabled = reference->GetEnabled();

	result = reference->SetActorIdleAnimation(idle);

	if (result && enabled && idle)
		SetActorIdleAnimation(reference, name, result->Lock());
}

void Game::net_UpdateInterior(const string& cell, bool spawn)
{
	CenterOnCell(cell, spawn);
}

void Game::net_UpdateExterior(unsigned int baseID, signed int x, signed int y, bool spawn)
{
	CenterOnWorld(baseID, x, y, spawn);
}

void Game::net_UpdateContext(Player::CellContext& context, bool spawn)
{
	if (spawn)
		spawnContext = context;

	auto player = GameFactory::GetObject<Player>(PLAYER_REFERENCE).get();
	unsigned int old_cell = player->GetNetworkCell();

	player->SetNetworkCell(context[0]);
	player->SetGameCell(context[0]);

	cellRefs.StartSession();
	cellContext.StartSession();

	(*cellRefs)[old_cell][ID_PLAYER].erase(PLAYER_REFERENCE);
	(*cellRefs)[context[0]][ID_PLAYER].insert(PLAYER_REFERENCE);

	sort(context.begin(), context.end());
	pair<vector<unsigned int>, vector<unsigned int>> diff;

	set_difference(context.begin(), context.end(), (*cellContext).begin(), (*cellContext).end(), inserter(diff.first, diff.first.begin()));
	set_difference((*cellContext).begin(), (*cellContext).end(), context.begin(), context.end(), inserter(diff.second, diff.second.begin()));

	*cellContext = context;
	CellRefs copy;

	for (const auto& cell : diff.second)
		if (cell)
			copy[cell] = (*cellRefs)[cell];

	for (const auto& cell : diff.first)
		if (cell)
			copy[cell] = (*cellRefs)[cell];

	cellContext.EndSession();
	cellRefs.EndSession();

	GameFactory::LeaveReference(player);

	for (const auto& cell : diff.second)
		if (cell)
			for (const auto& refs : copy[cell])
				for (unsigned int refID : refs.second)
					if (refID != PLAYER_REFERENCE)
					{
						auto reference = GameFactory::GetObject(refID);

						if (!reference)
							continue; // we don't have information about static refs yet. remove

						auto& object = reference.get();

						if (!object->IsPersistent() && object->SetEnabled(false))
							ToggleEnabled(object);
					}

	for (const auto& cell : diff.first)
		if (cell)
		{
			deletedObj.StartSession();
			vector<unsigned int> refIDs = move((*deletedObj)[cell]);
			deletedObj.EndSession();

			for (const auto& id : refIDs)
				RemoveObject(id);

			for (const auto& refs : copy[cell])
				for (unsigned int refID : refs.second)
					if (refID != PLAYER_REFERENCE)
					{
						auto reference = GameFactory::GetMultiple(vector<unsigned int>{refID, PLAYER_REFERENCE});

						if (!reference[0])
							continue; // we don't have information about static refs yet. remove

						auto& object = reference[0].get();

						if (object->SetEnabled(true))
							ToggleEnabled(object);

						if (object->SetGameCell(cell))
							MoveTo(object, reference[1].get(), true);

						object->Work();
					}

			uninitObj.StartSession();
			unordered_set<NetworkID> ids = move((*uninitObj)[cell]);
			uninitObj.EndSession();

			for (const auto& id : ids)
			{
				auto reference = GameFactory::GetObject(id);
				NewDispatch(reference.get());
			}
		}
}

void Game::net_UpdateConsole(bool enabled)
{
	Interface::StartDynamic();

	if (enabled)
		Interface::ExecuteCommand(Func::EnableKey, {RawParameter(ScanCode_Console)});
	else
		Interface::ExecuteCommand(Func::DisableKey, {RawParameter(ScanCode_Console)});

	Interface::EndDynamic();
}

void Game::net_UpdateWindowPos(const FactoryWindow& reference, const tuple<double, double, double, double>& pos)
{
	reference->SetPos(get<0>(pos), get<1>(pos), get<2>(pos), get<3>(pos));

	SetWindowPos(reference);
}

void Game::net_UpdateWindowSize(const FactoryWindow& reference, const tuple<double, double, double, double>& size)
{
	reference->SetSize(get<0>(size), get<1>(size), get<2>(size), get<3>(size));

	SetWindowSize(reference);
}

void Game::net_UpdateWindowVisible(const FactoryWindow& reference, bool visible)
{
	reference->SetVisible(visible);

	SetWindowVisible(reference);
}

void Game::net_UpdateWindowLocked(const FactoryWindow& reference, bool locked)
{
	reference->SetLocked(locked);

	SetWindowLocked(reference);
}

void Game::net_UpdateWindowText(const FactoryWindow& reference, const string& text)
{
	reference->SetText(text);

	SetWindowText(reference);
}

void Game::net_UpdateEditMaxLength(const FactoryEdit& reference, unsigned int length)
{
	reference->SetMaxLength(length);

	SetEditMaxLength(reference);
}

void Game::net_UpdateEditValidation(const FactoryEdit& reference, const std::string& validation)
{
	reference->SetValidation(validation);

	SetEditValidation(reference);
}

void Game::net_UpdateWindowMode(bool enabled)
{
	GUIMode = enabled;

	SetWindowMode();
}

void Game::net_UIMessage(const string& message, unsigned char emoticon)
{
	UIMessage(message, emoticon);
}

void Game::net_ChatMessage(const string& message)
{
	ChatMessage(message);
}

void Game::net_SetGlobalValue(unsigned int global, signed int value)
{
	globals.erase(global);
	globals.emplace(global, value);

	SetGlobalValue(global, value);
}

void Game::net_SetWeather(unsigned int weather)
{
	Game::weather = weather;

	SetWeather(weather);
}

void Game::net_SetBase(unsigned int base)
{
	Game::playerBase = base;
}

void Game::net_SetDeletedStatic(DeletedObjects&& deletedStatic)
{
	Game::deletedStatic.StartSession();
	(*Game::deletedStatic) = move(deletedStatic);
	Game::deletedStatic.EndSession();
}

void Game::GetPos(const FactoryObject& reference, unsigned char axis, double value)
{
	static bool update = false;

	bool result = static_cast<bool>(reference->SetGamePos(axis, value));

	if (reference->GetReference() == PLAYER_REFERENCE)
	{
		update |= result;

		if (axis == Axis_Z && update)
		{
			update = false;

			double X = reference->GetGamePos(Axis_X);
			double Y = reference->GetGamePos(Axis_Y);
			double Z = reference->GetGamePos(Axis_Z);

			reference->SetNetworkPos(Axis_X, X);
			reference->SetNetworkPos(Axis_Y, Y);
			reference->SetNetworkPos(Axis_Z, Z);

			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(reference->GetNetworkID(), X, Y, Z),
				HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::GetAngle(const FactoryObject& reference, unsigned char axis, double value)
{
	bool result = static_cast<bool>(reference->SetAngle(axis, value));

	if (result)
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(reference->GetNetworkID(), axis, value),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
		});
}

void Game::GetParentCell(const FactoryPlayer& player, unsigned int cell)
{
	bool result = static_cast<bool>(player->SetGameCell(cell));

	if (result)
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(player->GetNetworkID(), cell, 0.0, 0.0, 0.0),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
		});
}

/*
void Game::GetActorValue(const FactoryActor& reference, bool base, unsigned char index, double value)
{
	bool result;

	if (base)
		result = static_cast<bool>(reference->SetActorBaseValue(index, value));
	else
		result = static_cast<bool>(reference->SetActorValue(index, value));

	if (result)
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(reference->GetNetworkID(), base, index, value),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}
*/
void Game::GetActorState(const FactoryActor& reference, unsigned int idle, unsigned char moving, unsigned char weapon, unsigned char flags, bool sneaking)
{
	static pair<unsigned char, unsigned char> buf_weapon{AnimGroup_Idle, AnimGroup_Idle};

	unsigned char movingxy = flags & 0x03;
	bool result;

	if (moving == 0xFF)
		moving = AnimGroup_Idle;

	if (weapon == 0xFF)
		weapon = AnimGroup_Idle;

	result = (static_cast<bool>(reference->SetActorIdleAnimation(idle)) | static_cast<bool>(reference->SetActorMovingAnimation(moving)) | static_cast<bool>(reference->SetActorMovingXY(movingxy)) | static_cast<bool>(reference->SetActorSneaking(sneaking)));

	// workaround for occurences of wrong animation (case: 2HA weapons in AimIS mode, spuriously falls back to Aim for a frame)
	buf_weapon.first = buf_weapon.second;
	buf_weapon.second = weapon;

	if (buf_weapon.first == buf_weapon.second && static_cast<bool>(reference->SetActorWeaponAnimation(weapon)))
	{
		result = true;

		if (weapon == AnimGroup_Equip)
			reference->SetActorAlerted(true);
		else if (weapon == AnimGroup_Unequip)
			reference->SetActorAlerted(false);
	}

	if (result)
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(reference->GetNetworkID(), idle, moving, movingxy, reference->GetActorWeaponAnimation(), reference->GetActorAlerted(), sneaking, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::GetControl(const FactoryPlayer& reference, unsigned char control, unsigned char key)
{
	bool result = static_cast<bool>(reference->SetPlayerControl(control, key));

	if (result)
		Network::Queue({Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTROL>(reference->GetNetworkID(), control, key),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::GetActivate(const FactoryObject& reference, const FactoryObject& actor)
{
	Network::Queue({Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_ACTIVATE>(reference->GetNetworkID(), actor->GetNetworkID()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
	});
}

void Game::GetMessage(string message)
{
	if (message.empty())
		return;

	if (message.length() > MAX_CHAT_LENGTH)
		message.resize(MAX_CHAT_LENGTH);

	Network::Queue({Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_CHAT>(message),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
	});
}

void Game::GetWindowMode(bool enabled)
{
	GUIMode = enabled;

	if (GUIMode)
		DisablePlayerControls(true, true, true, false);
	else
		EnablePlayerControls();

	Network::Queue({Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_WMODE>(enabled),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
	});
}

void Game::GetWindowClick(string name)
{
	if (!name.compare(Button::CLOSE_BUTTON))
		Interface::SignalEnd();
	else
	{
		auto windows = GameFactory::GetObjectTypes<Window>(ALL_WINDOWS);

		for (auto& window : windows)
		{
			if (!window->GetLabel().compare(name))
			{
				Network::Queue({Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_WCLICK>(window->GetNetworkID()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
				});

				break;
			}
		}
	}
}

void Game::GetWindowText(string name, string text)
{
	auto windows = GameFactory::GetObjectTypes<Window>(ALL_WINDOWS);

	for (auto& window : windows)
	{
		if (!window->GetLabel().compare(name))
		{
			window->SetText(text);

			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_WTEXT>(window->GetNetworkID(), text),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
			});

			break;
		}
	}
}
