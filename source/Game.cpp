#include "Game.h"
#include "PacketFactory.h"
#include "GameFactory.h"

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
	unsigned short opcode = getFrom<unsigned short>(info.at(0));

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
				case Func_ForceRespawn:
				case Func_RemoveAllItemsEx:
				case Func_ScanContainer:
				case Func_CenterOnCell:
				case Func_CenterOnExterior:
				case Func_PlaceAtMeHealthPercent:
				case Func_GetCauseofDeath:
				case Func_GetRefCount:
				case Func_GetBaseObject:
				case Func_CenterOnWorld:
				case Func_Load:
					shared = Lockable::Poll(key);
					break;

				case Func_IsLimbGone:
				case Func_GetFirstRef:
				case Func_GetNextRef:
					break;

				default:
					Lockable::Retrieve(key);
			}
		}

		switch (opcode)
		{
			case Func_PlaceAtMeHealthPercent:
				FutureSet(shared, getFrom<unsigned int>(result));
				break;

			case Func_GetPos:
			{
				auto reference = GameFactory::GetObject(getFrom<unsigned int>(info.at(1)));
				GetPos(reference.get(), getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func_SetPos:
				break;

			case Func_GetAngle:
			{
				auto reference = GameFactory::GetObject(getFrom<unsigned int>(info.at(1)));
				GetAngle(reference.get(), getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func_SetAngle:
				break;

			case Func_GetActorValue:
			{
				auto reference = GameFactory::GetObject<Actor>(getFrom<unsigned int>(info.at(1)));
				GetActorValue(reference.get(), false, getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func_ForceActorValue:
				break;

			case Func_DamageActorValue:
				break;

			case Func_RestoreActorValue:
				break;

			case Func_GetBaseActorValue:
			{
				auto reference = GameFactory::GetObject<Actor>(getFrom<unsigned int>(info.at(1)));
				GetActorValue(reference.get(), true, getFrom<unsigned char>(info.at(2)), result);
				break;
			}

			case Func_SetActorValue:
				break;

			case Func_GetActorState:
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

			case Func_PlayGroup:
				break;

			case Func_PlayIdle:
				break;

			case Func_GetDead:
			{
				auto objects = GameFactory::GetMultiple<Actor>(vector<unsigned int>{getFrom<unsigned int>(info.at(1)), PLAYER_REFERENCE});
				GetDead(objects[0].get(), vaultcast<Player>(objects[1]).get(), result);
				break;
			}

			case Func_IsLimbGone:
				IsLimbGone(key, getFrom<unsigned int>(info.at(2)), result);
				break;

			case Func_GetCauseofDeath:
				FutureSet(shared, static_cast<signed char>(result));
				break;

			case Func_Kill:
				break;

			case Func_MoveTo:
				break;

			case Func_Enable:
				break;

			case Func_Disable:
				break;

			case Func_SetRestrained:
				break;

			case Func_SetAlert:
				break;

			case Func_SetForceSneak:
				break;

			case Func_AddItem:
				break;

			case Func_AddItemHealthPercent:
				break;

			case Func_RemoveItem:
				break;

			case Func_RemoveAllItems:
				break;

			case Func_EquipItem:
				break;

			case Func_UnequipItem:
				break;

			case Func_FireWeapon:
				break;

			case Func_EnablePlayerControls:
				break;

			case Func_DisablePlayerControls:
				break;

			case Func_SetINISetting:
				break;

			case Func_Lock:
				break;

			case Func_Unlock:
				break;

			case Func_SetOwnership:
				break;

			case Func_GetLocked:
			{
				auto reference = GameFactory::GetObject<Container>(getFrom<unsigned int>(info.at(1)));
				GetLocked(reference.get(), result);
				break;
			}

			case Func_GUIChat:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				GetMessage(string(reinterpret_cast<char*>(&data[0]), data.size()));
				delete &data;
				break;
			}

			case Func_GUIMode:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				GetWindowMode(data[0]);
				delete &data;
				break;
			}

			case Func_GUICreateWindow:
			case Func_GUICreateButton:
			case Func_GUICreateText:
			case Func_GUICreateEdit:
			case Func_GUIRemoveWindow:
			case Func_GUIPos:
			case Func_GUISize:
			case Func_GUIVisible:
			case Func_GUILocked:
				break;

			case Func_GUIText:
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

			case Func_GUIClick:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				GetWindowClick(string(reinterpret_cast<char*>(&data[0]), data.size()));
				delete &data;
				break;
			}

			case Func_SetGlobalValue:
				break;

			case Func_MarkForDelete:
				break;

			case Func_AgeRace:
				break;

			case Func_MatchRace:
				break;

			case Func_SexChange:
				break;

			case Func_ForceWeather:
				break;

			case Func_ScanContainer:
			{
				auto reference = GameFactory::GetObject<Container>(getFrom<unsigned int>(info.at(1)));
				vector<unsigned char>* data = getFrom<vector<unsigned char>*>(result);

				if (key)
					FutureSet(shared, GetScanContainer(reference.get(), *data));
				else
					ScanContainer(reference.get(), *data);

				delete data;
				break;
			}

			case Func_RemoveAllItemsEx:
			{
				auto reference = GameFactory::GetObject<Container>(getFrom<unsigned int>(info.at(1)));
				vector<unsigned char>* data = getFrom<vector<unsigned char>*>(result);
				GetRemoveAllItemsEx(reference.get(), *data);
				FutureSet(shared, true);
				delete data;
				break;
			}

			case Func_GetBaseObject:
				FutureSet(shared, getFrom<unsigned int>(result));
				break;

			case Func_GetRefCount:
				FutureSet(shared, static_cast<unsigned int>(result));
				break;

			case Func_SetRefCount:
				break;

			case Func_SetCurrentHealth:
				break;

			case Func_GetFirstRef:
				GetNextRef(key, getFrom<unsigned int>(result), getFrom<unsigned int>(info.at(1)));
				break;

			case Func_GetNextRef:
				GetNextRef(key, getFrom<unsigned int>(result));
				break;

			case Func_UIMessage:
				break;

			case Func_GetParentCell:
			{
				auto player = GameFactory::GetObject<Player>(getFrom<unsigned int>(info.at(1)));
				GetParentCell(player.get(), getFrom<unsigned int>(result));
				break;
			}

			case Func_EnableControl:
				break;

			case Func_DisableControl:
				break;

			case Func_GetControl:
			{
				auto self = GameFactory::GetObject<Player>(PLAYER_REFERENCE);
				GetControl(self.get(), getFrom<int>(info.at(1)), result);
				break;
			}

			case Func_DisableKey:
				break;

			case Func_EnableKey:
				break;

			case Func_CenterOnCell:
			case Func_CenterOnExterior:
			case Func_ForceRespawn:
			case Func_CenterOnWorld:
			case Func_Load:
				FutureSet(shared, true);
				break;

			case Func_SetName:
				break;

			default:
				throw VaultException("Unhandled function %04hX", opcode).stacktrace();
		}
	}
	else
	{
#ifdef VAULTMP_DEBUG
		debug.note("Command ", hex, opcode, " failed (");

		for (const auto& value : info)
			debug.note(*reinterpret_cast<const unsigned long long*>(&value), ", ");

		debug.print(")");
#endif

		switch (opcode)
		{
			case Func_PlaceAtMeHealthPercent:
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

	Interface::ExecuteCommand("GetControl", {RawParameter(API::RetrieveAllControls())});
	Interface::ExecuteCommand("DisableControl", {RawParameter(vector<unsigned char>{
		ControlCode_Quickload,
		ControlCode_Quicksave,
		ControlCode_VATS,
		ControlCode_Rest})});
	Interface::ExecuteCommand("DisableKey", {RawParameter(vector<unsigned int>{
		ScanCode_Escape,
		ScanCode_Console})});

	EnablePlayerControls();

	Interface::EndDynamic();

	Interface::StartSetup();

	Interface::SetupCommand("GetPos", {self_ref, Object::Param_Axis()});
	Interface::SetupCommand("GetPos", {Player::CreateFunctor(FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE), Object::Param_Axis()}, 30);
	//Interface::SetupCommand("GetPos", {Actor::CreateFunctor(FLAG_ENABLED | FLAG_ALIVE), Object::Param_Axis()}, 30);
	Interface::SetupCommand("GetAngle", {self_ref, RawParameter(vector<string> {API::RetrieveAxis_Reverse(Axis_X), API::RetrieveAxis_Reverse(Axis_Z)})});
	Interface::SetupCommand("GetActorState", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Player::CreateFunctor(FLAG_MOVCONTROLS, id)});
	Interface::SetupCommand("GetParentCell", {Player::CreateFunctor(FLAG_SELF | FLAG_ALIVE)}, 30);
	Interface::SetupCommand("ScanContainer", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED)}, 50);

	auto func = Player::CreateFunctor(FLAG_ENABLED | FLAG_ALIVE);
	func.connect(Actor::CreateFunctor(FLAG_ENABLED | FLAG_ALIVE));
	Interface::SetupCommand("GetDead", {move(func)}, 30);

	RawParameter health = RawParameter(vector<string>{
		API::RetrieveValue_Reverse(ActorVal_Health),
		API::RetrieveValue_Reverse(ActorVal_Head),
		API::RetrieveValue_Reverse(ActorVal_Torso),
		API::RetrieveValue_Reverse(ActorVal_LeftArm),
		API::RetrieveValue_Reverse(ActorVal_RightArm),
		API::RetrieveValue_Reverse(ActorVal_LeftLeg),
		API::RetrieveValue_Reverse(ActorVal_RightLeg),
		API::RetrieveValue_Reverse(ActorVal_Brain)});

	Interface::SetupCommand("GetActorValue", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), health}, 30);

	func = Player::CreateFunctor(FLAG_NOTSELF | FLAG_SELFALERT | FLAG_ENABLED | FLAG_ALIVE);
	func.connect(Actor::CreateFunctor(FLAG_SELFALERT | FLAG_ENABLED | FLAG_ALIVE));
	Interface::SetupCommand("GetActorValue", {move(func), health}, 30);

	// we could exclude health values here
	Interface::SetupCommand("GetActorValue", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Actor::Param_ActorValues()}, 100);
	Interface::SetupCommand("GetBaseActorValue", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Actor::Param_ActorValues()}, 200);

	func = Container::CreateFunctor(FLAG_ENABLED | FLAG_LOCKED);
	func.connect(Object::CreateFunctor(FLAG_ENABLED | FLAG_LOCKED));
	Interface::SetupCommand("GetLocked", {move(func)}, 200);

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

	Interface::ExecuteCommand("Load", {RawParameter(savegame)}, key);

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

	Interface::ExecuteCommand("CenterOnCell", {RawParameter(cell)}, key);

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

	Interface::ExecuteCommand("CenterOnExterior", {RawParameter(x), RawParameter(y)}, key);

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

	Interface::ExecuteCommand("CenterOnWorld", {RawParameter(baseID), RawParameter(x), RawParameter(y)}, key);

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

	Interface::ExecuteCommand("SetINISetting", {RawParameter(key), RawParameter(value)});

	Interface::EndDynamic();
}

void Game::SetGlobalValue(unsigned int global, signed int value)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("SetGlobalValue", {RawParameter(global), RawParameter(value)});

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
		auto _reference = GameFactory::GetObject(id);
		auto& reference = _reference.get();

		if (!reference->IsPersistent() || reference->GetReference() == PLAYER_REFERENCE)
		{
			cellRefs.StartSession();
			(*cellRefs)[reference->GetNetworkCell()][reference.GetType()].erase(reference->GetReference());
			cellRefs.EndSession();

			if (reference->GetReference() != PLAYER_REFERENCE)
				reference->SetReference(0x00000000);
			else
			{
				reference->SetNetworkCell(spawnContext[0]);
				reference->SetGameCell(spawnContext[0]);
			}
		}

		NewDispatch(reference);
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

	Interface::ExecuteCommand("UIMessage", {RawParameter(message), RawParameter(emoticon)});

	Interface::EndDynamic();
}

void Game::ChatMessage(const string& message)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("GUIChat", {RawParameter(message)});

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

void Game::NewItem_(FactoryItem& reference)
{
	NetworkID id = reference->GetItemContainer();

	if (id)
		throw VaultException("Cannot create item %llu which is bound to a Container (%llu)", reference->GetNetworkID(), id).stacktrace();

	NewObject_(reference);
	SetRefCount(reference);
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

		Interface::ExecuteCommand("MarkForDelete", {reference->GetReferenceParam()});

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

	Interface::ExecuteCommand("MarkForDelete", {RawParameter(refID)});

	Interface::EndDynamic();
}

void Game::NewWindow(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	if (reference->GetLabel().empty())
	{
		reference->SetLabel(Utils::toString(reference->GetNetworkID()));

		Interface::ExecuteCommand("GUICreateWindow", {RawParameter(reference->GetLabel())});
	}

	WindowPos(reference);
	WindowSize(reference);
	WindowVisible(reference);
	WindowLocked(reference);
	WindowText(reference);

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

		Interface::ExecuteCommand("GUICreateButton", {RawParameter(Utils::toString(reference->GetParentWindow())), RawParameter(reference->GetLabel())});
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

		Interface::ExecuteCommand("GUICreateText", {RawParameter(Utils::toString(reference->GetParentWindow())), RawParameter(reference->GetLabel())});
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

		Interface::ExecuteCommand("GUICreateEdit", {RawParameter(Utils::toString(reference->GetParentWindow())), RawParameter(reference->GetLabel())});
	}

	NewWindow(reference);

	Interface::EndDynamic();
}

void Game::PlaceAtMe(const FactoryObject& reference, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	PlaceAtMe(reference->GetReference(), baseID, condition, count, key);
}

void Game::PlaceAtMe(unsigned int refID, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("PlaceAtMeHealthPercent", {RawParameter(refID), RawParameter(baseID), RawParameter(condition), RawParameter(count)}, key);

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
		Interface::ExecuteCommand("Enable", {RawParameter(refID), RawParameter(true)});
	else
		Interface::ExecuteCommand("Disable", {RawParameter(refID), RawParameter(false)});

	Interface::EndDynamic();
}

void Game::DeleteObject(FactoryObject& reference)
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

void Game::DeleteWindow(FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("GUIRemoveWindow", {RawParameter(reference->GetLabel())});

	Interface::EndDynamic();

	GameFactory::DestroyInstance(reference);
}

unsigned int Game::GetBase(unsigned int refID)
{
	auto store = make_shared<Shared<unsigned int>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("GetBaseObject", {RawParameter(refID)}, key);

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

	if (name.empty())
		return;

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetName", {reference->GetReferenceParam(), RawParameter(name)});

	Interface::EndDynamic();
}

void Game::SetRestrained(const FactoryActor& reference, bool restrained)
{
	//bool restrained = actor->GetActorRestrained();

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetRestrained", {reference->GetReferenceParam(), RawParameter(restrained)});

	Interface::EndDynamic();
}

void Game::SetPos(const FactoryObject& reference)
{
	if (!reference->HasValidCoordinates())
		return;

	Lockable* key = nullptr;

	Interface::StartDynamic();

	key = reference->SetGamePos(Axis_X, reference->GetNetworkPos(Axis_X));

	Interface::ExecuteCommand("SetPos", {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(reference->GetNetworkPos(Axis_X))}, key ? key->Lock() : 0);

	key = reference->SetGamePos(Axis_Y, reference->GetNetworkPos(Axis_Y));

	Interface::ExecuteCommand("SetPos", {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Y)), RawParameter(reference->GetNetworkPos(Axis_Y))}, key ? key->Lock() : 0);

	key = reference->SetGamePos(Axis_Z, reference->GetNetworkPos(Axis_Z));

	Interface::ExecuteCommand("SetPos", {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(reference->GetNetworkPos(Axis_Z))}, key ? key->Lock() : 0);

	Interface::EndDynamic();
}

void Game::SetAngle(const FactoryObject& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("SetAngle", {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(reference->GetAngle(Axis_X))});

	double value = reference->GetAngle(Axis_Z);
	auto actor = vaultcast<Actor>(reference);

	if (actor)
	{
		if (actor->GetActorMovingXY() == 0x01)
			AdjustZAngle(value, -45.0);
		else if (actor->GetActorMovingXY() == 0x02)
			AdjustZAngle(value, 45.0);
	}

	Interface::ExecuteCommand("SetAngle", {reference->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(value)});

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

	Interface::ExecuteCommand("MoveTo", move(param_MoveTo), key);

	Interface::EndDynamic();
}

void Game::SetLock(const FactoryObject& reference, unsigned int key)
{
	unsigned int lock = reference->GetLockLevel();

	if (lock == Lock_Broken) // workaround: can't set lock to broken, so set it to impossible
		lock = 255;

	auto* object = reference.operator->();

	auto func = [object, lock](unsigned int key)
	{
		Interface::StartDynamic();

		if (lock != Lock_Unlocked)
			Interface::ExecuteCommand("Lock", {object->GetReferenceParam(), RawParameter(lock)}, key);
		else
			Interface::ExecuteCommand("Unlock", {object->GetReferenceParam()}, key);

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

		Interface::ExecuteCommand("SetOwnership", {object->GetReferenceParam(), RawParameter(owner)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::SetActorValue(const FactoryActor& reference, bool base, unsigned char index, unsigned int key)
{
	Interface::StartDynamic();

	if (base)
		Interface::ExecuteCommand("SetActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(reference->GetActorBaseValue(index))}, key);
	else
		Interface::ExecuteCommand("ForceActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(reference->GetActorValue(index))}, key);

	Interface::EndDynamic();
}

void Game::DamageActorValue(const FactoryActor& reference, unsigned char index, double value, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("DamageActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

	Interface::EndDynamic();
}

void Game::RestoreActorValue(const FactoryActor& reference, unsigned char index, double value, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("RestoreActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

	Interface::EndDynamic();
}

void Game::SetActorSneaking(const FactoryActor& reference, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("SetForceSneak", {reference->GetReferenceParam(), RawParameter(reference->GetActorSneaking())}, key);

	Interface::EndDynamic();
}

void Game::SetActorAlerted(const FactoryActor& reference, unsigned int key)
{
	// really need to introduce restrained state in Actor class

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetAlert", {reference->GetReferenceParam(), RawParameter(reference->GetActorAlerted())}, key);

	Interface::EndDynamic();
}

void Game::SetActorAnimation(const FactoryActor& reference, unsigned char anim, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("PlayGroup", {reference->GetReferenceParam(), RawParameter(API::RetrieveAnim_Reverse(anim)), RawParameter(true)}, key);

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

	Interface::ExecuteCommand("PlayIdle", {reference->GetReferenceParam(), RawParameter(anim)}, key);

	Interface::EndDynamic();
}

void Game::SetActorRace(const FactoryActor& reference, signed int delta_age, unsigned int key)
{
	unsigned int baseID = reference->GetBase();
	unsigned int race = reference->GetActorRace();

	// set only once per base
	if (baseRaces[baseID] == race || race == UINT_MAX) // creature test
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	baseRaces[baseID] = race;

	Interface::StartDynamic();

	Interface::ExecuteCommand("MatchRace", {reference->GetReferenceParam(), RawParameter(race)}, delta_age ? 0 : key);

	if (delta_age)
		Interface::ExecuteCommand("AgeRace", {reference->GetReferenceParam(), RawParameter(delta_age)}, key);

	Interface::EndDynamic();
}

void Game::SetActorFemale(const FactoryActor& reference, unsigned int key)
{
	if (reference->GetActorRace() == UINT_MAX) // creature test
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	Interface::StartDynamic();

	Interface::ExecuteCommand("SexChange", {reference->GetReferenceParam(), RawParameter(reference->GetActorFemale())}, key);

	Interface::EndDynamic();
}

void Game::KillActor(const FactoryActor& reference, unsigned short limbs, signed char cause, unsigned int key)
{
	Interface::StartDynamic();

	// maybe add valid killer later
	if (limbs)
	{
		unsigned int j = 0;

		for (unsigned int i = 1; i <= limbs; i <<= 1, ++j)
			if (limbs & i)
				Interface::ExecuteCommand("Kill", {reference->GetReferenceParam(), reference->GetReferenceParam(), RawParameter(j), RawParameter(cause)}, ((i << 1) > limbs) ? key : 0x00);
	}
	else
		Interface::ExecuteCommand("Kill", {reference->GetReferenceParam(), reference->GetReferenceParam(), RawParameter(Limb_None), RawParameter(cause)}, key);

	Interface::EndDynamic();
}

void Game::FireWeapon(const FactoryActor& reference, unsigned int weapon, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("FireWeapon", {reference->GetReferenceParam(), RawParameter(weapon)}, key);

	Interface::EndDynamic();
}

void Game::AddItem(const FactoryContainer& reference, const FactoryItem& item, unsigned int key)
{
	AddItem(reference, item->GetBase(), item->GetItemCount(), item->GetItemCondition(), item->GetItemSilent(), key);
}

void Game::AddItem(const FactoryContainer& reference, unsigned int baseID, unsigned int count, double condition, bool silent, unsigned int key)
{
	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && reference->GetReference() == PLAYER_REFERENCE)
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	auto* container = reference.operator->();

	auto func = [container, baseID, count, condition, silent](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand("AddItemHealthPercent", {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(condition / 100), RawParameter(silent)}, key);

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
	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && reference->GetReference() == PLAYER_REFERENCE)
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	auto* container = reference.operator->();

	auto func = [container, baseID, count, silent](unsigned int key)
	{
		Interface::StartDynamic();

		Interface::ExecuteCommand("RemoveItem", {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(silent)}, key);

		Interface::EndDynamic();
	};

	DelayOrExecute(reference, func, key);
}

void Game::RemoveAllItems(const FactoryContainer& reference, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveAllItems", {reference->GetReferenceParam()}, key);

	Interface::EndDynamic();
}

void Game::RemoveAllItemsEx(FactoryContainer& reference)
{
	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveAllItemsEx", {reference->GetReferenceParam()}, key);

	Interface::EndDynamic();

	NetworkID id = reference->GetNetworkID();
	GameFactory::LeaveReference(reference);

	try
	{
		store->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Obtaining of all items of %llu for RemoveAllItemsEx failed (%s)", id, e.what()).stacktrace();
	}
}

void Game::SetRefCount(const FactoryItem& reference, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("SetRefCount", {reference->GetReferenceParam(), RawParameter(reference->GetItemCount())}, key);

	Interface::EndDynamic();
}

void Game::SetCurrentHealth(const FactoryItem& reference, unsigned int health, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("SetCurrentHealth", {reference->GetReferenceParam(), RawParameter(health)}, key);

	Interface::EndDynamic();
}

unsigned int Game::GetRefCount(unsigned int refID)
{
	auto store = make_shared<Shared<unsigned int>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("GetRefCount", {RawParameter(refID)}, key);

	Interface::EndDynamic();

	unsigned int count;

	try
	{
		count = store->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Obtaining of reference count of refID %08X (%s)", refID, e.what()).stacktrace();
	}

	return count;
}

void Game::EquipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key)
{
	EquipItem(reference, item->GetBase(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::EquipItem(const FactoryActor& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && reference->GetReference() == PLAYER_REFERENCE)
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	Interface::StartDynamic();

	Interface::ExecuteCommand("EquipItem", {reference->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::UnequipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key)
{
	UnequipItem(reference, item->GetBase(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::UnequipItem(const FactoryActor& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && reference->GetReference() == PLAYER_REFERENCE)
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	Interface::StartDynamic();

	Interface::ExecuteCommand("UnequipItem", {reference->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::WindowPos(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("GUIPos", {RawParameter(reference->GetLabel()), RawParameter(get<0>(reference->GetPos())), RawParameter(get<1>(reference->GetPos())), RawParameter(get<2>(reference->GetPos())), RawParameter(get<3>(reference->GetPos()))});

	Interface::EndDynamic();
}

void Game::WindowSize(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("GUISize", {RawParameter(reference->GetLabel()), RawParameter(get<0>(reference->GetSize())), RawParameter(get<1>(reference->GetSize())), RawParameter(get<2>(reference->GetSize())), RawParameter(get<3>(reference->GetSize()))});

	Interface::EndDynamic();
}

void Game::WindowVisible(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("GUIVisible", {RawParameter(reference->GetLabel()), RawParameter(reference->GetVisible())});

	Interface::EndDynamic();
}

void Game::WindowLocked(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("GUILocked", {RawParameter(reference->GetLabel()), RawParameter(reference->GetLocked())});

	Interface::EndDynamic();
}

void Game::WindowText(const FactoryWindow& reference)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("GUIText", {RawParameter(reference->GetLabel()), RawParameter(reference->GetText())});

	Interface::EndDynamic();
}

void Game::WindowMode()
{
	if (GUIMode)
		DisablePlayerControls(true, true, true, false);
	else
		EnablePlayerControls();

	Interface::StartDynamic();

	Interface::ExecuteCommand("GUIMode", {RawParameter(GUIMode)});

	Interface::EndDynamic();
}

Game::CellDiff Game::ScanCell(unsigned int type)
{
	auto store = make_shared<Shared<CellDiff>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("GetFirstRef", {RawParameter(type)}, key);

	Interface::EndDynamic();

	CellDiff diff;

	try
	{
		diff = store->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Scan of player cell with type %d failed (%s)", type, e.what()).stacktrace();
	}

	return diff;
}

pair<ItemList::NetDiff, ItemList::GameDiff> Game::ScanContainer(FactoryContainer& reference)
{
	auto store = make_shared<Shared<pair<ItemList::NetDiff, ItemList::GameDiff>>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("ScanContainer", {reference->GetReferenceParam()}, key);

	Interface::EndDynamic();

	NetworkID id = reference->GetNetworkID();
	GameFactory::LeaveReference(reference);

	pair<ItemList::NetDiff, ItemList::GameDiff> diff;

	try
	{
		diff = store->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		// for resolving a weird bug with promise already satisfied, investigate
		VaultException("Scan of container inventory %llu failed (%s)", id, e.what()).stacktrace();
	}

	return diff;
}

void Game::EnablePlayerControls(bool movement, bool pipboy, bool fighting, bool pov, bool looking, bool rollover, bool sneaking)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("EnablePlayerControls", {RawParameter(movement), RawParameter(pipboy), RawParameter(fighting), RawParameter(pov), RawParameter(looking), RawParameter(rollover), RawParameter(sneaking)});

	Interface::EndDynamic();
}

void Game::DisablePlayerControls(bool movement, bool pipboy, bool fighting, bool pov, bool looking, bool rollover, bool sneaking)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("DisablePlayerControls", {RawParameter(movement), RawParameter(pipboy), RawParameter(fighting), RawParameter(pov), RawParameter(looking), RawParameter(rollover), RawParameter(sneaking)});

	Interface::EndDynamic();
}

void Game::SetWeather(unsigned int weather)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("ForceWeather", {RawParameter(weather), RawParameter(1)});

	Interface::EndDynamic();
}

void Game::ForceRespawn()
{
	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("ForceRespawn", {}, key);

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
		SetLock(reference, result->Lock());
}

void Game::net_SetOwner(const FactoryObject& reference, unsigned int owner)
{
	if (reference->SetOwner(owner))
		SetOwner(reference);
}

void Game::net_SetItemCount(const FactoryItem& reference, unsigned int count)
{
	if (reference->GetItemContainer())
		return;

	if (reference->SetItemCount(count))
		SetRefCount(reference);
}

void Game::net_SetItemCondition(const FactoryItem& reference, double condition, unsigned int health)
{
	if (reference->GetItemContainer())
		return;

	if (reference->SetItemCondition(condition))
		SetCurrentHealth(reference, health);
}

void Game::net_UpdateContainer(FactoryContainer& reference, const ItemList::NetDiff& ndiff, const ItemList::NetDiff& gdiff)
{
	Lockable* result;

	// cleaner solution here

	ItemList::ContainerDiff diff = ItemList::ToContainerDiff(ndiff);
	NetworkID id = reference->GetNetworkID();
	auto container = reference.operator ->();
	GameFactory::LeaveReference(reference);

	while (!(result = container->getLock()));

	reference = GameFactory::GetObject<Container>(id).get();

	unsigned int key = result->Lock();

	ItemList::GameDiff _gdiff = reference->IL.ApplyDiff(diff);

	for (const auto& diff : _gdiff)
	{
		if (diff.second.equipped)
		{
			if (diff.second.equipped > 0)
				EquipItem(vaultcast<Actor>(reference).get(), diff.first, diff.second.silent, diff.second.stick, result->Lock());
			else if (diff.second.equipped < 0)
				UnequipItem(vaultcast<Actor>(reference).get(), diff.first, diff.second.silent, diff.second.stick, result->Lock());
		}
		else if (diff.second.count > 0)
			AddItem(reference, diff.first, diff.second.count, diff.second.condition, diff.second.silent, result->Lock());
		else if (diff.second.count < 0)
			RemoveItem(reference, diff.first, abs(diff.second.count), diff.second.silent, result->Lock());

		//else
		// new condition, can't handle yet
	}

	result->Unlock(key);

	GameFactory::LeaveReference(reference);

	for (const auto& id : gdiff.first)
	{
		auto reference = GameFactory::GetObject<Item>(id);

		if (!reference)
		{
#ifdef VAULTMP_DEBUG
			debug.print("WARNING (net_ContainerUpdate): item ", dec, id, " not found. Has it already been deleted? ", GameFactory::IsDeleted(id) ? "YES" : "NO");
#endif
			continue;
		}

		DeleteObject(reference.get());
	}

	for (const auto& packet : gdiff.second)
	{
		NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet.get());
		auto reference = GameFactory::GetObject<Item>(id);
		reference->SetReference(0x00000000);
		NewItem(reference.get());
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
				DamageActorValue(reference, index, diff, result->Lock());
			else if (diff > 0.00)
				RestoreActorValue(reference, index, diff, result->Lock());
		}
		else
			SetActorValue(reference, base, index, result->Lock());
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
			KillActor(reference, limbs, cause, result->Lock());
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
		Interface::ExecuteCommand("EnableKey", {RawParameter(ScanCode_Console)});
	else
		Interface::ExecuteCommand("DisableKey", {RawParameter(ScanCode_Console)});

	Interface::EndDynamic();
}

void Game::net_UpdateWindowPos(const FactoryWindow& reference, const tuple<double, double, double, double>& pos)
{
	reference->SetPos(get<0>(pos), get<1>(pos), get<2>(pos), get<3>(pos));

	WindowPos(reference);
}

void Game::net_UpdateWindowSize(const FactoryWindow& reference, const tuple<double, double, double, double>& size)
{
	reference->SetSize(get<0>(size), get<1>(size), get<2>(size), get<3>(size));

	WindowSize(reference);
}

void Game::net_UpdateWindowVisible(const FactoryWindow& reference, bool visible)
{
	reference->SetVisible(visible);

	WindowVisible(reference);
}

void Game::net_UpdateWindowLocked(const FactoryWindow& reference, bool locked)
{
	reference->SetLocked(locked);

	WindowLocked(reference);
}

void Game::net_UpdateWindowText(const FactoryWindow& reference, const string& text)
{
	reference->SetText(text);

	WindowText(reference);
}

void Game::net_UpdateWindowMode(bool enabled)
{
	GUIMode = enabled;

	WindowMode();
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

void Game::GetDead(const FactoryActor& reference, const FactoryPlayer&, bool dead)
{
	/*if (actor != self && !self->GetActorAlerted())
	{
	    // "bug death"

	    return;
	}*/

	bool result = static_cast<bool>(reference->SetActorDead(dead));

	if (result)
	{
		if (dead)
		{
			NetworkID id = reference->GetNetworkID();

			AsyncDispatch([id]
			{
				try
				{
					unsigned int refID = GameFactory::GetObject<Actor>(id)->GetReference();
					unsigned int key;
					unsigned short limbs;
					signed char cause;

					{
						auto store = make_shared<Shared<unsigned short>>();
						key = Lockable::Share(store);

						Interface::StartDynamic();

						Interface::ExecuteCommand("IsLimbGone", {RawParameter(refID), RawParameter(vector<unsigned char>{
							Limb_Torso,
							Limb_Head1,
							Limb_Head2,
							Limb_LeftArm1,
							Limb_LeftArm2,
							Limb_RightArm1,
							Limb_RightArm2,
							Limb_LeftLeg1,
							Limb_LeftLeg2,
							Limb_LeftLeg3,
							Limb_RightLeg1,
							Limb_RightLeg2,
							Limb_RightLeg3,
							Limb_Brain,
							Limb_Weapon})}, key);

						Interface::EndDynamic();

						try
						{
							limbs = store->get_future(chrono::seconds(5));
						}
						catch (exception& e)
						{
							throw VaultException("Obtaining of limb data of %08X failed (%s)", refID, e.what()).stacktrace();
						}
					}

					{
						auto store = make_shared<Shared<signed char>>();
						key = Lockable::Share(store);

						Interface::StartDynamic();

						Interface::ExecuteCommand("GetCauseofDeath", {RawParameter(refID)}, key);

						Interface::EndDynamic();

						try
						{
							cause = store->get_future(chrono::seconds(5));
						}
						catch (exception& e)
						{
							throw VaultException("Obtaining of cause of death of %08X failed (%s)", refID, e.what()).stacktrace();
						}
					}

					Network::Queue({Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, true, limbs, cause),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
					});
				}
				catch (...) {}
			});
		}
		else
		{
			Network::Queue({Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(reference->GetNetworkID(), false, 0, 0),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::IsLimbGone(unsigned int key, unsigned char limb, bool gone)
{
	bool last_limb = limb == Limb_Weapon;

	shared_ptr<Lockable> shared = Lockable::Poll(key, last_limb).lock();
	Lockable* locked = shared.get();

	if (locked == nullptr)
		throw VaultException("Storage has expired").stacktrace();

	Shared<unsigned short>* store = dynamic_cast<Shared<unsigned short>*>(locked);

	if (store == nullptr)
		throw VaultException("Storage is corrupted").stacktrace();

	(**store) |= (static_cast<unsigned short>(gone) << limb);

	if (last_limb)
		store->set_promise();
}

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

void Game::ScanContainer(const FactoryContainer& reference, const vector<unsigned char>& data)
{
	Lockable* result;

	if ((result = reference->getLock()))
	{
		unsigned int key = result->Lock();

		auto _result = GetScanContainer(reference, data);

		if (!_result.first.first.empty() || !_result.first.second.empty())
		{
			auto& ndiff = _result.first;
			auto& gdiff = _result.second;

			gdiff.remove_if([](const pair<unsigned int, ItemList::Diff>& diff) { return !diff.second.count; });

			if (!gdiff.empty())
			{
				// lambda can't capture by move :(

				auto _ndiff = make_shared<ItemList::NetDiff>(move(ndiff));
				NetworkID id = reference->GetNetworkID();

				JobDispatch(chrono::milliseconds(0), [_ndiff, gdiff, key, result, id]() mutable
				{
					try
					{
						{
							// ALL_CONTAINERS: removed due to weird behaviour. fix
							auto reference = GetContext(ID_CONTAINER);
							auto player = GameFactory::GetObject<Player>(id).get();

							double X = player->GetGamePos(Axis_X);
							double Y = player->GetGamePos(Axis_Y);
							double Z = player->GetGamePos(Axis_Z);

							GameFactory::LeaveReference(player);

							for (unsigned int ref : reference)
							{
								auto container = GameFactory::GetObject<Container>(ref).get();
								NetworkID id_ = container->GetNetworkID();

								if (!container->IsNearPoint(X, Y, Z, 500.0))
									continue;

								auto result = Game::ScanContainer(container);

								if (!result.first.first.empty() || !result.first.second.empty())
								{
									Network::Queue({Network::CreateResponse(
										PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id_, result.first, ItemList::NetDiff()),
										HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
									});

									for (const auto& _diff : result.second)
										for (auto& diff : gdiff)
											if (_diff.first == diff.first && _diff.second.count)
												if ((_diff.second.count > 0 && diff.second.count < 0 && (_diff.second.count + diff.second.count) >= 0) ||
													(_diff.second.count < 0 && diff.second.count > 0 && (_diff.second.count + diff.second.count) <= 0))
													diff.second.count += _diff.second.count;
								}
							}

							gdiff.remove_if([](const pair<unsigned int, ItemList::Diff>& diff) { return !diff.second.count; });
						}

						ItemList::NetDiff ndiff;

						if (!gdiff.empty())
						{
							CellDiff cdiff = ScanCell(FormType_Inventory);
							map<unsigned int, pair<ItemList::GameDiff::iterator, list<pair<unsigned int, unsigned int>>>> found;

							const auto best_match = [](decltype(found)::value_type& found)
							{
								auto& data = found.second.second;
								data.sort();

								unsigned int count = abs(found.second.first->second.count);
								pair<unsigned int, vector<pair<unsigned int, unsigned int>>> result;
								result.first = UINT_MAX;

								do
								{
									unsigned int num = 0;
									signed int i = count;

									for (const auto& ref : data)
									{
										i -= ref.second;
										++num;

										if (i < 0)
											num = 0;

										if (i <= 0)
											break;
									}

									if (!i && result.first > num)
									{
										auto it = data.begin(); advance(it, num);
										result.second.assign(data.begin(), it);
										result.first = num;
									}
								} while (next_permutation(data.begin(), data.end()));

								return result;
							};

							for (unsigned int refID : cdiff.first)
							{
								unsigned int baseID = Game::GetBase(refID);

								auto it = find_if(gdiff.begin(), gdiff.end(), [baseID](const pair<unsigned int, ItemList::Diff>& diff) { return diff.first == baseID; });

								if (it != gdiff.end())
								{
									if (it->second.count < 0)
									{
										unsigned int count = Game::GetRefCount(refID);

										if (static_cast<signed int>(count) + it->second.count <= 0)
										{
											auto& data = found[baseID];
											data.first = it;
											data.second.emplace_back(refID, count);
										}
#ifdef VAULTMP_DEBUG
										else
											debug.print("Item match (drop): could not match ", hex, refID, " (baseID: ", baseID, "), count ", dec, count);
#endif
									}
								}
#ifdef VAULTMP_DEBUG
								else
									debug.print("Item match (drop): could not match ", hex, refID, " (baseID: ", baseID, ") at all");
#endif
							}

							if (!found.empty())
							{
								double X, Y, Z;
								unsigned int cell;

								{
									FactoryContainer reference = GameFactory::GetObject<Container>(id).get();

									static const double spawn_offset = 100.0;

									// maybe better synchronically obtain XYZ for correctness
									auto offset = reference->GetOffset(spawn_offset);

									X = offset.first;
									Y = offset.second;
									Z = reference->GetGamePos(Axis_Z) + 70.0;

									cell = reference->GetGameCell();
								}

								for (auto& _found : found)
								{
									auto result = best_match(_found);

#ifdef VAULTMP_DEBUG
									unsigned int count = abs(_found.second.first->second.count);

									if (!result.second.empty())
										debug.print("Player dropped ", hex, _found.first, " (count ", dec, count, ", stacks ", result.second.size(), ")");
									else
										debug.print("Could not find a matching set for item drop ", hex, _found.first, " (count ", dec, ")");
#endif

									for (const auto& _result : result.second)
									{
										NetworkID id = GameFactory::CreateInstance(ID_ITEM, _result.first, _found.first);
										FactoryItem reference = GameFactory::GetObject<Item>(id).get();

										reference->SetEnabled(true);
										reference->SetGamePos(Axis_X, X);
										reference->SetGamePos(Axis_Y, Y);
										reference->SetGamePos(Axis_Z, Z);
										reference->SetNetworkPos(Axis_X, X);
										reference->SetNetworkPos(Axis_Y, Y);
										reference->SetNetworkPos(Axis_Z, Z);
										reference->SetNetworkCell(cell);
										reference->SetGameCell(cell);

										reference->SetItemCount(_result.second);
										reference->SetItemCondition(_found.second.first->second.condition);

										ndiff.second.emplace_back(reference->toPacket());
									}

									gdiff.erase(_found.second.first);
								}
							}

							found.clear();

							for (unsigned int refID : cdiff.second)
							{
								auto _reference = GameFactory::GetObject<Item>(refID);

								if (!_reference)
								{
#ifdef VAULTMP_DEBUG
									debug.print("Item match (pickup): could not find ", hex, refID);
#endif
									continue;
								}

								auto& reference = _reference.get();

								unsigned int baseID = reference->GetBase();

								auto it = find_if(gdiff.begin(), gdiff.end(), [baseID](const pair<unsigned int, ItemList::Diff>& diff) { return diff.first == baseID; });

								if (it != gdiff.end())
								{
									if (it->second.count > 0)
									{
										unsigned int count = reference->GetItemCount();

										if (it->second.count - static_cast<signed int>(count) >= 0)
										{
											auto& data = found[baseID];
											data.first = it;
											data.second.emplace_back(refID, count);
										}
#ifdef VAULTMP_DEBUG
										else
											debug.print("Item match (pickup): could not match ", hex, refID, " (baseID: ", baseID, "), count ", dec, count);
#endif
									}
								}
#ifdef VAULTMP_DEBUG
								else
									debug.print("Item match (pickup): could not match ", hex, refID, " (baseID: ", baseID, ") at all");
#endif
							}

							if (!found.empty())
							{
								for (auto& _found : found)
								{
									auto result = best_match(_found);

#ifdef VAULTMP_DEBUG
									unsigned int count = abs(_found.second.first->second.count);

									if (!result.second.empty())
										debug.print("Player picked up ", hex, _found.first, " (count ", dec, count, ", stacks ", result.second.size(), ")");
									else
										debug.print("Could not find a matching set for item pickup ", hex, _found.first, " (count ", dec, count, ")");
#endif

									for (const auto& _result : result.second)
									{
										NetworkID id = GameFactory::LookupNetworkID(_result.first);
										ndiff.first.emplace_back(id);
										GameFactory::DestroyInstance(id);
									}

									gdiff.erase(_found.second.first);
								}
							}

#ifdef VAULTMP_DEBUG
							for (const auto& _gdiff : gdiff)
								debug.print("Could not match drop / pickup ", hex, _gdiff.first, " (count ", dec, _gdiff.second.count, ")");
#endif
						}

						Network::Queue({Network::CreateResponse(
							PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, *_ndiff, ndiff),
							HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
						});
					}
					catch (...) {}

					result->Unlock(key);
				});

				key = 0x00000000;
			}
			else
				Network::Queue({Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(reference->GetNetworkID(), ndiff, ItemList::NetDiff()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
				});
		}

		if (key)
			result->Unlock(key);
	}
}

pair<ItemList::NetDiff, ItemList::GameDiff> Game::GetScanContainer(const FactoryContainer& reference, const vector<unsigned char>& data)
{
	pair<ItemList::NetDiff, ItemList::GameDiff> result;

#pragma pack(push, 1)
	struct ItemInfo
	{
		unsigned int baseID;
		unsigned int count;
		unsigned int equipped;
		double condition;
	};
#pragma pack(pop)

	const ItemInfo* items = reinterpret_cast<const ItemInfo*>(&data[0]);
	unsigned int count = data.size() / sizeof(ItemInfo);

	FactoryContainer temp = GameFactory::GetObject<Container>(GameFactory::CreateInstance(ID_CONTAINER, 0x00000000)).get();

	for (unsigned int i = 0; i < count; ++i)
	{
		FactoryItem item = GameFactory::GetObject<Item>(GameFactory::CreateInstance(ID_ITEM, items[i].baseID)).get();
		item->SetItemCount(items[i].count);
		item->SetItemEquipped(static_cast<bool>(items[i].equipped));
		item->SetItemCondition(items[i].condition);
		temp->IL.AddItem(item->GetNetworkID());
	}

	auto diff = reference->IL.Compare(temp->GetNetworkID());

	if (!diff.first.empty() || !diff.second.empty())
	{
		result.first = ItemList::ToNetDiff(diff);
		result.second = reference->IL.ApplyDiff(diff);
	}

	GameFactory::DestroyInstance(temp);

	return result;
}

void Game::GetRemoveAllItemsEx(const FactoryContainer& reference, const vector<unsigned char>& data)
{
#pragma pack(push, 1)
	struct ItemInfo
	{
		unsigned int baseID;
		unsigned int count;
		unsigned int equipped;
		double condition;
	};
#pragma pack(pop)

	const ItemInfo* items = reinterpret_cast<const ItemInfo*>(&data[0]);
	unsigned int count = data.size() / sizeof(ItemInfo);

	for (unsigned int i = 0; i < count; ++i)
		RemoveItem(reference, items[i].baseID, items[i].count, true);
}

void Game::GetLocked(const FactoryContainer& reference, unsigned int lock)
{
	switch (lock)
	{
		case 0:
			if (reference->SetLockLevel(Lock_Unlocked))
				Network::Queue({Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_LOCK>(reference->GetNetworkID(), Lock_Unlocked),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
				});
			break;
		case 1:
			break;
		case 2:
			if (reference->SetLockLevel(Lock_Broken))
				Network::Queue({Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_LOCK>(reference->GetNetworkID(), Lock_Broken),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
				});
			break;
	}
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

void Game::GetNextRef(unsigned int key, unsigned int refID, unsigned int type)
{
	static bool first = true;
	static unsigned int cell;
	static unsigned int _type;
	static set<unsigned int> data;

	static unordered_map<unsigned int, unsigned int> typemap {
		{FormType_Inventory, ID_ITEM},
		{FormType_Actor, ID_ACTOR},
	};

	if (first)
	{
		FactoryPlayer reference = GameFactory::GetObject<Player>(PLAYER_REFERENCE).get();
		cell = reference->GetGameCell();
		_type = typemap[type];
		first = false;
	}

	if (refID)
	{
		data.insert(refID);

		Interface::StartDynamic();

		Interface::ExecuteCommand("GetNextRef", {}, key);

		Interface::EndDynamic();
	}
	else
	{
		shared_ptr<Lockable> shared = Lockable::Poll(key).lock();
		Lockable* locked = shared.get();

		if (locked == nullptr)
			throw VaultException("Storage has expired").stacktrace();

		Shared<CellDiff>* store = dynamic_cast<Shared<CellDiff>*>(locked);

		if (store == nullptr)
			throw VaultException("Storage is corrupted").stacktrace();

		CellDiff diff;

		cellRefs.StartSession();

		auto& refs = (*cellRefs)[cell][_type];

		set_difference(data.begin(), data.end(), refs.begin(), refs.end(), inserter(diff.first, diff.first.begin()));
		set_difference(refs.begin(), refs.end(), data.begin(), data.end(), inserter(diff.second, diff.second.begin()));

		refs.swap(data);

		cellRefs.EndSession();

		data.clear();

		store->set(diff);
		store->set_promise();

		first = true;
	}
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
	static const char* exit_button = "closeBTN";

	if (!name.compare(exit_button))
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
