#include "Game.hpp"
#include "Shared.hpp"

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
bool Game::GUIMode;

#ifdef VAULTMP_DEBUG
DebugInput<Game> Game::debug;
#endif

void Game::AdjustZAngle(float& Z, float diff)
{
	Z += diff;

	if (Z > 360.0f)
		Z -= 360.0f;
	else if (Z < 0.00f)
		Z += 360.0f;
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
				if (key)
					FutureSet(shared, getFrom<unsigned int>(result));
				break;

			case Func::GetPosAngle:
			{
				float* data = getFrom<float*>(result);

				GameFactory::Operate<Object, EXCEPTION_FACTORY_VALIDATED>(getFrom<unsigned int>(info.at(1)), [&info, data](FactoryObject& object) {
					GetAngle(object, data[0], data[1], data[2]);
					GetPos(object, data[3], data[4], data[5]);
				});
				break;
			}

			case Func::GetActivate:
			{
				if (!result)
					break;

				unsigned int refID = *getFrom<unsigned int*>(result);

				GameFactory::Operate<Reference, RETURN_FACTORY_VALIDATED>({refID, PLAYER_REFERENCE}, [result](FactoryReferences& references) {
					GetActivate(references[0], references[1]);
				});
				break;
			}

			case Func::GetFireWeapon:
				GameFactory::Operate<Player, EXCEPTION_FACTORY_VALIDATED>(PLAYER_REFERENCE, [](FactoryPlayer& player) {
					GetFireWeapon(player);
				}); break;

			case Func::GetActorState:
				GameFactory::Operate<Actor, EXCEPTION_FACTORY_VALIDATED>(getFrom<unsigned int>(info.at(1)), [&info, result](FactoryActor& actor) mutable {
					GetActorState(actor,
						*reinterpret_cast<unsigned int*>(&result),
						*reinterpret_cast<unsigned char*>(((unsigned) &result) + 4),
						*reinterpret_cast<unsigned char*>(((unsigned) &result) + 6),
						*reinterpret_cast<unsigned char*>(((unsigned) &result) + 5),
						*reinterpret_cast<unsigned char*>(((unsigned) &result) + 7));
				}); break;

			case Func::GetParentCell:
				GameFactory::Operate<Player, EXCEPTION_FACTORY_VALIDATED>(getFrom<unsigned int>(info.at(1)), [result](FactoryPlayer& player) {
					GetParentCell(player, getFrom<unsigned int>(result));
				}); break;

			case Func::GetControl:
				GameFactory::Operate<Player, EXCEPTION_FACTORY_VALIDATED>(PLAYER_REFERENCE, [&info, result](FactoryPlayer& player) {
					GetControl(player, getFrom<int>(info.at(1)), result);
				}); break;

			case Func::GUIChat:
			{
				if (!result)
					break;

				GetMessage(getFrom<char*>(result));
				break;
			}

			case Func::GUIMode:
			{
				if (!result)
					break;

				GetWindowMode(*getFrom<bool*>(result));
				break;
			}

			case Func::GUIText:
			{
				if (!result)
					break;

				char* data = getFrom<char*>(result);
				string name = data;
				string text = data + name.length() + 1;
				GetWindowText(move(name), move(text));
				break;
			}

			case Func::GUIClick:
			{
				if (!result)
					break;

				GetWindowClick(getFrom<char*>(result));
				break;
			}

			case Func::GUICheckbox:
			{
				if (!result)
					break;

				char* data = getFrom<char*>(result);
				string name = data;
				GetCheckboxSelected(name, *reinterpret_cast<bool*>(data + name.length() + 1));
				break;
			}

			case Func::GUISelect:
			{
				if (!result)
					break;

				char* data = getFrom<char*>(result);
				string name = data;
				data += name.length() + 1;

				vector<const char*> selections;

				while (*data)
				{
					selections.emplace_back(data);
					data += strlen(data) + 1;
				}

				GetListboxSelections(name, selections);
				break;
			}

			case Func::GUIReturn:
			{
				if (!result)
					break;

				GetWindowReturn(getFrom<char*>(result));
				break;
			}

			case Func::CenterOnCell:
			case Func::CenterOnWorld:
			case Func::ForceRespawn:
				FutureSet(shared, true);
				break;

			case Func::SetPosFast:
			case Func::SetAngle:
			case Func::ForceActorValue:
			case Func::DamageActorValue:
			case Func::RestoreActorValue:
			case Func::SetActorValue:
			case Func::PlayGroup:
			case Func::PlayIdle:
			case Func::Kill:
			case Func::MoveTo:
			case Func::Enable:
			case Func::Disable:
			case Func::SetRestrained:
			case Func::SetAlert:
			case Func::SetForceSneak:
			case Func::AddItemHealthPercent:
			case Func::RemoveItem:
			case Func::EquipItem:
			case Func::UnequipItem:
			case Func::FireWeapon:
			case Func::EnablePlayerControls:
			case Func::DisablePlayerControls:
			case Func::SetINISetting:
			case Func::Lock:
			case Func::Unlock:
			case Func::SetOwnership:
			case Func::Activate:
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
			case Func::GUICreateCheckbox:
			case Func::GUICreateRadio:
			case Func::GUIRadioGroup:
			case Func::GUICreateListbox:
			case Func::GUICreateItem:
			case Func::GUIRemoveItem:
			case Func::GUISelectChange:
			case Func::GUISelectText:
			case Func::GUISelectMulti:
			case Func::SetGlobalValue:
			case Func::MarkForDelete:
			case Func::AgeRace:
			case Func::MatchRace:
			case Func::SexChange:
			case Func::ForceWeather:
			case Func::SetRefCount:
			case Func::SetCurrentHealth:
			case Func::UIMessage:
			case Func::DisableControl:
			case Func::EnableControl:
			case Func::DisableKey:
			case Func::EnableKey:
			case Func::SetName:
			case Func::PlaceAtMePrepare:
			case Func::PlaySound_:
			case Func::PlaySound3D:
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
	return GameFactory::Operate<Player>(PLAYER_REFERENCE, [&password](Player* player) {
		return NetworkResponse{{
			PacketFactory::Create<pTypes::ID_GAME_AUTH>(player->GetName(), password),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		};
	});
}

void Game::Initialize()
{
	cellRefs.Operate([](CellRefs& cellRefs) { cellRefs.clear(); });
	cellContext.Operate([](Player::CellContext& cellContext) { cellContext = {{0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}}; });
	uninitObj.Operate([](UninitializedObjects& uninitObj) { uninitObj.clear(); });
	deletedObj.Operate([](DeletedObjects& deletedObj) { deletedObj.clear(); });
	deletedStatic.Operate([](DeletedObjects& deletedStatic) { deletedStatic.clear(); });

	baseRaces.clear();
	globals.clear();
	weather = 0x00000000;
	playerBase = 0x00000000;
	spawnFunc = SpawnFunc();
	spawnContext = {{0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};
	GUIMode = false;
}

void Game::Startup()
{
	NetworkID id = GameFactory::Lookup<Player>(PLAYER_REFERENCE);

	static RawParameter self_ref(GameFactory::Operate<Player>(id, [](Player* player) {
		return player->GetReferenceParam();
	}));

	Interface::Dynamic([]() {
		SetINISetting("bSaveOnInteriorExteriorSwitch:GamePlay", "0");
		SetINISetting("bSaveOnTravel:GamePlay", "0");
		SetINISetting("bSaveOnWait:GamePlay", "0");
		SetINISetting("bSaveOnRest:GamePlay", "0");
		SetGlobalValue(Global_TimeScale, 0);

		Interface::ExecuteCommand(Func::GetControl, {RawParameter(API::controls)});

		ToggleControl(false, ControlCode_Quickload);
		ToggleControl(false, ControlCode_VATS);
		ToggleControl(false, ControlCode_Rest);
		ToggleControl(false, ControlCode_MenuMode);
		ToggleKey(false, ScanCode_Escape);
		ToggleKey(false, ScanCode_Console);

		EnablePlayerControls();
	});

	Interface::Setup([id]() {
		Interface::SetupCommand(Func::GetPosAngle, {self_ref});
		Interface::SetupCommand(Func::GetPosAngle, {self_ref});
		Interface::SetupCommand(Func::GetPosAngle, {self_ref});
		Interface::SetupCommand(Func::GetPosAngle, {self_ref});
		Interface::SetupCommand(Func::GetPosAngle, {self_ref});
		Interface::SetupCommand(Func::GetPosAngle, {Player::CreateFunctor(FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE)}, 30);
		Interface::SetupCommand(Func::GetActorState, {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Player::CreateFunctor(FLAG_MOVCONTROLS, id)});
		Interface::SetupCommand(Func::GetParentCell, {Player::CreateFunctor(FLAG_SELF | FLAG_ALIVE)}, 30);
	});

	while (!GameFactory::Operate<Player>(id, [](Player* player) {
		return get<0>(player->GetGamePos());
	})) this_thread::sleep_for(chrono::milliseconds(10));
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
/*
void Game::AsyncDispatch(function<void()>&& func)
{
	thread t(move(func));
	t.detach();
}

void Game::JobDispatch(chrono::milliseconds&& time, function<void()>&& func)
{
	Interface::PushJob(chrono::steady_clock::now() + time, move(func));
}
*/
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

	Interface::Dynamic([&savegame, key]() {
		Interface::ExecuteCommand(Func::Load, {RawParameter(savegame)}, key);
	});

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

	Interface::Dynamic([&cell, key]() {
		Interface::ExecuteCommand(Func::CenterOnCell, {RawParameter(cell)}, key);
	});

	try
	{
		store->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of cell %s failed (%s)", cell.c_str(), e.what()).stacktrace();
	}

	if (first)
		LoadEnvironment();

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

	Interface::Dynamic([x, y, key]() {
		Interface::ExecuteCommand(Func::CenterOnExterior, {RawParameter(x), RawParameter(y)}, key);
	});

	try
	{
		store->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of cell (%d,%d) failed (%s)", x, y, e.what()).stacktrace();
	}

	if (first)
		LoadEnvironment();

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

	Interface::Dynamic([baseID, x, y, key]() {
		Interface::ExecuteCommand(Func::CenterOnWorld, {RawParameter(baseID), RawParameter(x), RawParameter(y)}, key);
	});

	try
	{
		store->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of world (%08X,%d,%d) failed (%s)", baseID, x, y, e.what()).stacktrace();
	}

	if (first)
		LoadEnvironment();

	// ready state
}

void Game::SetINISetting(const string& key, const string& value)
{
	Interface::Dynamic([&key, &value]() {
		Interface::ExecuteCommand(Func::SetINISetting, {RawParameter(key), RawParameter(value)});
	});
}

void Game::SetGlobalValue(unsigned int global, signed int value)
{
	Interface::Dynamic([global, value]() {
		Interface::ExecuteCommand(Func::SetGlobalValue, {RawParameter(global), RawParameter(value)});
	});
}

void Game::LoadEnvironment()
{
	for (const auto& global : globals)
		SetGlobalValue(global.first, global.second);

	if (weather)
		SetWeather(weather);

	uninitObj.Operate([](UninitializedObjects& uninitObj) {
		uninitObj.clear();
	});

	deletedObj.Operate([](DeletedObjects& deletedObj) {
		deletedStatic.Operate([&deletedObj](DeletedObjects& deletedStatic) {
			deletedObj = deletedStatic;
		});

		for (unsigned int cell : spawnContext)
			if (cell)
			{
				for (unsigned int refID : deletedObj[cell])
					RemoveObject(refID);

				deletedObj[cell].clear();
			}
	});

	vector<NetworkID> reference = GameFactory::GetByType(ALL_OBJECTS);

	for (NetworkID id : reference)
		GameFactory::Operate<Object, EXCEPTION_FACTORY_VALIDATED>(id, [](FactoryObject& object) {
			if (!object->IsPersistent() || object->GetReference() == PLAYER_REFERENCE)
			{
				cellRefs.Operate([&object](CellRefs& cellRefs) {
					cellRefs[object->GetNetworkCell()][object.GetType()].erase(object->GetReference());
				});

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

void Game::NewDispatch(FactoryObject& reference)
{
	unsigned int type = reference.GetType();

	switch (type)
	{
		case ID_OBJECT:
			NewObject(reference);
			break;

		case ID_ITEM:
		{
			auto item = vaultcast_swap<Item>(move(reference)).get();

			if (!item->GetItemContainer())
				NewItem(item);
			break;
		}

		case ID_CONTAINER:
			NewContainer(vaultcast_swap<Container>(move(reference)).get());
			break;

		case ID_ACTOR:
			NewActor(vaultcast_swap<Actor>(move(reference)).get());
			break;

		case ID_PLAYER:
			NewPlayer(vaultcast_swap<Player>(move(reference)).get());
			break;

		default:
			throw VaultException("Can't create object of unknown type %08X", type).stacktrace();
	}
}

void Game::UIMessage(const string& message, unsigned char emoticon)
{
	Interface::Dynamic([&message, emoticon]() {
		Interface::ExecuteCommand(Func::UIMessage, {RawParameter(message), RawParameter(emoticon)});
	});
}

void Game::ChatMessage(const string& message)
{
	Interface::Dynamic([&message]() {
		Interface::ExecuteCommand(Func::GUIChat, {RawParameter(message)});
	});
}

void Game::NewObject(FactoryObject& reference)
{
	if (IsInContext(reference->GetNetworkCell()))
		NewObject_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.Operate([&reference](UninitializedObjects& uninitObj) {
			auto& sub = uninitObj[reference->GetNetworkCell()];
			sub.insert(reference->IsPersistent() ? sub.begin() : sub.end(), reference->GetNetworkID());
		});
	}
}

void Game::NewObject_(FactoryObject& reference)
{
	reference->Release();
	unsigned int refID = reference->GetReference();

	if (!reference->GetReference())
	{
		auto store = make_shared<Shared<unsigned int>>();
		unsigned int key = Lockable::Share(store);

		float condition;

		{
			auto item = vaultcast<Item>(reference);
			condition = item ? (item->GetItemCondition() / 100.0) : 1.00;
		}

		unsigned int baseID = reference->GetBase();
		unsigned int anchorID = GetAnchor(reference->GetNetworkCell());

		Interface::Dynamic([&reference, anchorID, baseID, condition, key]() {
			const auto& angle = reference->GetAngle();
			const auto& pos = reference->GetNetworkPos();

			Interface::ExecuteCommand(Func::PlaceAtMePrepare, {RawParameter(get<0>(angle)), RawParameter(get<1>(angle)), RawParameter(get<2>(angle)), RawParameter(get<0>(pos)), RawParameter(get<1>(pos)), RawParameter(get<2>(pos))});

			PlaceAtMe(anchorID, baseID, condition, 1, key);
		});

		NetworkID id = reference->GetNetworkID();

		GameFactory::Free(reference);

		try
		{
			refID = store->get_future(chrono::seconds(5));
		}
		catch (exception& e)
		{
			throw VaultException("Object creation with baseID %08X and NetworkID %llu failed (%s)", baseID, id, e.what()).stacktrace();
		}

		reference = GameFactory::Get<Object>(id).get();
		reference->SetReference(refID);
	}

	reference->SetEnabled(true);
	reference->SetGameCell(reference->GetNetworkCell());

	cellRefs.Operate([&reference, refID](CellRefs& cellRefs) {
		cellRefs[reference->GetNetworkCell()][reference.GetType()].insert(refID);
	});

	SetName(reference);

	//if (reference->GetLockLevel() != Lock_Unlocked)
		SetLock(reference);

	if (reference->GetOwner())
		SetOwner(reference);

	// maybe more
}

void Game::NewVolatile(FactoryObject& reference, unsigned int baseID, float aX, float aY, float aZ)
{
	if (!IsInContext(reference->GetNetworkCell()))
		return;

	Interface::Dynamic([&reference, baseID, aX, aY, aZ]() {
		const auto& pos = reference->GetNetworkPos();

		Interface::ExecuteCommand(Func::PlaceAtMePrepare, {RawParameter(aX), RawParameter(aY), RawParameter(aZ), RawParameter(get<0>(pos)), RawParameter(get<1>(pos)), RawParameter(get<2>(pos))});

		PlaceAtMe(reference, baseID, 1.00f, 1);
	});
}

void Game::NewItem(FactoryItem& reference)
{
	NetworkID item = reference->GetNetworkID();
	NetworkID container = reference->GetItemContainer();

	if (container)
	{
		GameFactory::Free(reference);

		GameFactory::Operate<Container, EXCEPTION_FACTORY_VALIDATED>(container, [item](FactoryContainer& container) {
			container->AddItem(item);

			GameFactory::Operate<Item, EXCEPTION_FACTORY_VALIDATED>(item, [&container](FactoryItem& item) {
				AddItem(container, item);

				if (item->GetItemEquipped() && vaultcast_test<Actor>(container))
					GameFactory::Operate<Actor, EXCEPTION_FACTORY_VALIDATED>(container->GetNetworkID(), [&item](FactoryActor& actor) {
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

			uninitObj.Operate([&reference](UninitializedObjects& uninitObj) {
				auto& sub = uninitObj[reference->GetNetworkCell()];
				sub.insert(reference->IsPersistent() ? sub.begin() : sub.end(), reference->GetNetworkID());
			});
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
}

void Game::NewContainer(FactoryContainer& reference)
{
	if (IsInContext(reference->GetNetworkCell()))
		NewContainer_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.Operate([&reference](UninitializedObjects& uninitObj) {
			auto& sub = uninitObj[reference->GetNetworkCell()];
			sub.insert(reference->IsPersistent() ? sub.begin() : sub.end(), reference->GetNetworkID());
		});
	}
}

void Game::NewContainer_(FactoryContainer& reference)
{
	NewObject_(reference);
	auto items = GameFactory::Get<Item>(reference->GetItemList());

	for (auto& item : items)
	{
		AddItem(reference, item.get());

		if (item->GetItemEquipped() && vaultcast_test<Actor>(reference))
			EquipItem(vaultcast<Actor>(reference).get(), item.get());
	}
}

void Game::NewActor(FactoryActor& reference)
{
	if (IsInContext(reference->GetNetworkCell()))
		NewActor_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.Operate([&reference](UninitializedObjects& uninitObj) {
			auto& sub = uninitObj[reference->GetNetworkCell()];
			sub.insert(reference->IsPersistent() ? sub.begin() : sub.end(), reference->GetNetworkID());
		});
	}
}

void Game::NewActor_(FactoryActor& reference)
{
	NewContainer_(reference);

	for (const auto& value : API::values)
	{
		SetActorValue(reference, true, value.second);
		SetActorValue(reference, false, value.second);
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
			KillActor(reference);
	}
}

void Game::NewPlayer(FactoryPlayer& reference)
{
	if (IsInContext(reference->GetNetworkCell()) || reference->GetReference() == PLAYER_REFERENCE)
		NewPlayer_(reference);
	else
	{
		reference->SetEnabled(false);

		uninitObj.Operate([&reference](UninitializedObjects& uninitObj) {
			auto& sub = uninitObj[reference->GetNetworkCell()];
			sub.insert(reference->IsPersistent() ? sub.begin() : sub.end(), reference->GetNetworkID());
		});
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

		Interface::Dynamic([&reference]() {
			Interface::ExecuteCommand(Func::MarkForDelete, {reference->GetReferenceParam()});
		});
	}
	else
		deletedObj.Operate([&reference](DeletedObjects& deletedObj) {
			deletedObj[reference->GetGameCell()].emplace_back(reference->GetReference());
		});

	cellRefs.Operate([&reference](CellRefs& cellRefs) {
		cellRefs[reference->GetNetworkCell()][reference.GetType()].erase(reference->GetReference());
	});
}

void Game::RemoveObject(unsigned int refID)
{
	ToggleEnabled(refID, false);

	Interface::Dynamic([refID]() {
		Interface::ExecuteCommand(Func::MarkForDelete, {RawParameter(refID)});
	});
}

void Game::NewWindow(const FactoryWindow& reference)
{
	Interface::Dynamic([&reference]() {
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
	});
}

void Game::NewButton(const FactoryButton& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::Dynamic([&reference]() {
		if (reference->GetLabel().empty())
		{
			reference->SetLabel(Utils::toString(reference->GetNetworkID()));

			Interface::ExecuteCommand(Func::GUICreateButton, {RawParameter(reference->GetParentWindow()), RawParameter(reference->GetLabel())});
		}

		NewWindow(reference);
	});
}

void Game::NewText(const FactoryText& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::Dynamic([&reference]() {
		if (reference->GetLabel().empty())
		{
			reference->SetLabel(Utils::toString(reference->GetNetworkID()));

			Interface::ExecuteCommand(Func::GUICreateText, {RawParameter(reference->GetParentWindow()), RawParameter(reference->GetLabel())});
		}

		NewWindow(reference);
	});
}

void Game::NewEdit(const FactoryEdit& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::Dynamic([&reference]() {
		if (reference->GetLabel().empty())
		{
			reference->SetLabel(Utils::toString(reference->GetNetworkID()));

			Interface::ExecuteCommand(Func::GUICreateEdit, {RawParameter(reference->GetParentWindow()), RawParameter(reference->GetLabel())});
		}

		NewWindow(reference);

		SetEditMaxLength(reference);
		SetEditValidation(reference);
	});
}

void Game::NewCheckbox(const FactoryCheckbox& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::Dynamic([&reference]() {
		if (reference->GetLabel().empty())
		{
			reference->SetLabel(Utils::toString(reference->GetNetworkID()));

			Interface::ExecuteCommand(Func::GUICreateCheckbox, {RawParameter(reference->GetParentWindow()), RawParameter(reference->GetLabel())});
		}

		NewWindow(reference);

		SetCheckboxSelected(reference);
	});
}

void Game::NewRadioButton(const FactoryRadioButton& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::Dynamic([&reference]() {
		if (reference->GetLabel().empty())
		{
			reference->SetLabel(Utils::toString(reference->GetNetworkID()));

			Interface::ExecuteCommand(Func::GUICreateRadio, {RawParameter(reference->GetParentWindow()), RawParameter(reference->GetLabel()), RawParameter(reference->GetGroup())});
		}

		NewWindow(reference);

		SetRadioButtonSelected(reference);
	});
}

void Game::NewListItem(FactoryListItem& reference)
{
	NetworkID item = reference->GetNetworkID();
	NetworkID list = reference->GetItemContainer();

	GameFactory::Free(reference);

	GameFactory::Operate<List>(list, [item](List* list) {
		list->AddItem(item);

		GameFactory::Operate<ListItem>(item, [list](ListItem* listitem) {
			Interface::Dynamic([listitem, list]() {
				Interface::ExecuteCommand(Func::GUICreateItem, {RawParameter(list->GetLabel()), RawParameter(listitem->GetNetworkID()), RawParameter(listitem->GetText())});
			});
		});
	});
}

void Game::NewList(const FactoryList& reference)
{
	if (!reference->GetParentWindow())
		throw VaultException("Window %llu requires a parent", reference->GetNetworkID());

	Interface::Dynamic([&reference]() {
		if (reference->GetLabel().empty())
		{
			reference->SetLabel(Utils::toString(reference->GetNetworkID()));

			Interface::ExecuteCommand(Func::GUICreateListbox, {RawParameter(reference->GetParentWindow()), RawParameter(reference->GetLabel())});
		}

		NewWindow(reference);

		SetListMultiSelect(reference);
	});

	GameFactory::Operate<ListItem>(reference->GetItemList(), [&reference](ListItems& listitems) {
		Interface::Dynamic([&listitems, &reference]() {
			for (auto listitem : listitems)
				Interface::ExecuteCommand(Func::GUICreateItem, {RawParameter(reference->GetLabel()), RawParameter(listitem->GetNetworkID()), RawParameter(listitem->GetText())});
		});
	});
}

void Game::PlaceAtMe(const FactoryObject& reference, unsigned int baseID, float condition, unsigned int count, unsigned int key)
{
	PlaceAtMe(reference->GetReference(), baseID, condition, count, key);
}

void Game::PlaceAtMe(unsigned int refID, unsigned int baseID, float condition, unsigned int count, unsigned int key)
{
	Interface::Dynamic([refID, baseID, condition, count, key]() {
		Interface::ExecuteCommand(Func::PlaceAtMeHealthPercent, {RawParameter(refID), RawParameter(baseID), RawParameter(condition), RawParameter(count)}, key);
	});
}

void Game::ToggleEnabled(const FactoryObject& reference)
{
	ToggleEnabled(reference->GetReference(), reference->GetEnabled());
}

void Game::ToggleEnabled(unsigned int refID, bool enabled)
{
	Interface::Dynamic([refID, enabled]() {
		if (enabled)
			Interface::ExecuteCommand(Func::Enable, {RawParameter(refID), RawParameter(true)});
		else
			Interface::ExecuteCommand(Func::Disable, {RawParameter(refID), RawParameter(false)});
	});
}

void Game::DestroyObject(FactoryObject& reference, bool silent)
{
	NetworkID id = reference->GetNetworkID();

	NetworkID container = GameFactory::Operate<Item, RETURN_VALIDATED>(id, [](Item* item) {
		return item->GetItemContainer();
	});

	if (container)
	{
		GameFactory::Free(reference);

		GameFactory::Operate<Container, EXCEPTION_FACTORY_VALIDATED>(container, [id, silent](FactoryContainer& container) {
			container->RemoveItem(id);

			GameFactory::Operate<Item>(id, [&container, silent](Item* item) {
				RemoveItem(container, item->GetBase(), item->GetItemCount(), silent);

				// Game always removes equipped item first - workaround (is this really always the case?)
				NetworkID equipped = container->IsEquipped(item->GetBase());

				if (equipped && vaultcast_test<Actor>(container))
					GameFactory::Operate<Actor, EXCEPTION_FACTORY_VALIDATED>(container->GetNetworkID(), [equipped](FactoryActor& actor) {
						GameFactory::Operate<Item>(equipped, [&actor](Item* item) {
							EquipItem(actor, item->GetBase(), item->GetItemCondition(), false, item->GetItemStick());
						});
					});
			});

			GameFactory::Destroy(id);
		});
	}
	else
	{
		RemoveObject(reference);

		uninitObj.Operate([&reference](UninitializedObjects& uninitObj) {
			auto& sub = uninitObj[reference->GetNetworkCell()];
			auto it = find(sub.begin(), sub.end(), reference->GetNetworkID());

			if (it != sub.end())
				sub.erase(it);
		});

		if (reference->IsPersistent())
			deletedStatic.Operate([&reference](DeletedObjects& deletedStatic) {
				deletedStatic[reference->GetNetworkCell()].emplace_back(reference->GetReference());
			});

		GameFactory::Destroy(reference);
	}
}

void Game::DestroyWindow(FactoryWindow& reference)
{
	GameFactory::Operate<List, RETURN_VALIDATED>(reference->GetNetworkID(), [](List* list) {
		GameFactory::Operate<ListItem>(list->GetItemList(), [list](ListItems& items) {
			Interface::Dynamic([&items, list]() {
				for (auto item : items)
				{
					list->RemoveItem(item->GetNetworkID());
					Interface::ExecuteCommand(Func::GUIRemoveItem, {RawParameter(list->GetLabel()), RawParameter(item->GetNetworkID())});
				}
			});
		});
	});

	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUIRemoveWindow, {RawParameter(reference->GetLabel())});
	});

	GameFactory::Destroy(reference);
}

void Game::DestroyListItem(FactoryListItem& reference)
{
	NetworkID item = reference->GetNetworkID();
	NetworkID list = reference->GetItemContainer();

	GameFactory::Free(reference);

	GameFactory::Operate<List>(list, [item](List* list) {
		list->RemoveItem(item);

		GameFactory::Operate<ListItem>(item, [list](ListItem* listitem) {
			Interface::Dynamic([listitem, list]() {
				Interface::ExecuteCommand(Func::GUIRemoveItem, {RawParameter(list->GetLabel()), RawParameter(listitem->GetNetworkID())});
			});
		});

		GameFactory::Destroy(item);
	});
}

unsigned int Game::GetBase(unsigned int refID)
{
	auto store = make_shared<Shared<unsigned int>>();
	unsigned int key = Lockable::Share(store);

	Interface::Dynamic([refID, key]() {
		Interface::ExecuteCommand(Func::GetBaseObject, {RawParameter(refID)}, key);
	});

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

	DelayOrExecute(reference, [object, name](unsigned int) {
		Interface::Dynamic([object, &name]() {
			Interface::ExecuteCommand(Func::SetName, {object->GetReferenceParam(), RawParameter(name)});
		});
	}, 0);
}

void Game::SetRestrained(const FactoryActor& reference, bool restrained)
{
	Interface::Dynamic([&reference, restrained]() {
		Interface::ExecuteCommand(Func::SetRestrained, {reference->GetReferenceParam(), RawParameter(restrained)});
	});
}

void Game::Activate(const FactoryReference& reference, const FactoryReference& actor)
{
	Interface::Dynamic([&reference, &actor]() {
		Interface::ExecuteCommand(Func::Activate, {reference->GetReferenceParam(), actor->GetReferenceParam()});
	});
}

void Game::SetPos(const FactoryObject& reference)
{
	if (!reference->HasValidCoordinates())
		return;

	Interface::Dynamic([&reference]() {
		const auto& pos = reference->GetNetworkPos();

		Lockable* key = reference->SetGamePos(pos);

		Interface::ExecuteCommand(Func::SetPosFast, {reference->GetReferenceParam(), RawParameter(get<0>(pos)), RawParameter(get<1>(pos)), RawParameter(get<2>(pos))}, key ? key->Lock() : 0);
	});
}

void Game::SetAngle(const FactoryObject& reference)
{
	Interface::Dynamic([&reference]() {
		const auto& angle = reference->GetAngle();

		Interface::ExecuteCommand(Func::SetAngle, {reference->GetReferenceParam(), RawParameter(Axis_X), RawParameter(get<0>(angle))});

		float value = get<2>(angle);
		auto actor = vaultcast<Actor>(reference);

		if (actor)
		{
			if (actor->GetActorMovingXY() == 0x01)
				AdjustZAngle(value, -45.0f);
			else if (actor->GetActorMovingXY() == 0x02)
				AdjustZAngle(value, 45.0f);
		}

		Interface::ExecuteCommand(Func::SetAngle, {reference->GetReferenceParam(), RawParameter(Axis_Z), RawParameter(value)});
	});
}

void Game::MoveTo(const FactoryObject& reference, const FactoryObject& object, bool cell, unsigned int key)
{
	Interface::Dynamic([&reference, &object, cell, key]() {
		ParamContainer param_MoveTo{reference->GetReferenceParam(), object->GetReferenceParam()};

		if (cell)
		{
			const auto& pos_1 = reference->GetNetworkPos();
			const auto& pos_2 = object->GetNetworkPos();

			param_MoveTo.emplace_back(get<0>(pos_1) - get<0>(pos_2));
			param_MoveTo.emplace_back(get<1>(pos_1) - get<1>(pos_2));
			param_MoveTo.emplace_back(get<2>(pos_1) - get<2>(pos_2));
		}

		Interface::ExecuteCommand(Func::MoveTo, move(param_MoveTo), key);
	});
}

void Game::SetLock(const FactoryObject& reference, unsigned int key)
{
	auto* object = reference.operator->();
	unsigned int lock = object->GetLockLevel();

	if (lock == Lock_Broken) // workaround: can't set lock to broken, so set it to impossible
		lock = Lock_Impossible;

	DelayOrExecute(reference, [object, lock](unsigned int key) {
		Interface::Dynamic([object, lock, key]() {
			if (lock != Lock_Unlocked)
				Interface::ExecuteCommand(Func::Lock, {object->GetReferenceParam(), RawParameter(lock)}, key);
			else
				Interface::ExecuteCommand(Func::Unlock, {object->GetReferenceParam()}, key);
		});
	}, key);
}

void Game::SetOwner(const FactoryObject& reference, unsigned int key)
{
	auto* object = reference.operator->();
	unsigned int owner = object->GetOwner();

	if (owner == playerBase)
		owner = PLAYER_BASE;

	DelayOrExecute(reference, [object, owner](unsigned int key) {
		Interface::Dynamic([object, owner, key]() {
			Interface::ExecuteCommand(Func::SetOwnership, {object->GetReferenceParam(), RawParameter(owner)}, key);
		});
	}, key);
}

void Game::PlaySound(const FactoryObject& reference, unsigned int sound)
{
	if (!IsInContext(reference->GetNetworkCell()))
		return;

	Interface::Dynamic([&reference, sound]() {
		Interface::ExecuteCommand(Func::PlaySound3D, {reference->GetReferenceParam(), RawParameter(sound)});
	});
}

void Game::SetActorValue(const FactoryActor& reference, bool base, unsigned char index, unsigned int key)
{
	auto* actor = reference.operator->();
	float value = base ? actor->GetActorBaseValue(index) : actor->GetActorValue(index);

	DelayOrExecute(reference, [actor, base, index, value](unsigned int key) {
		Interface::Dynamic([actor, base, index, value, key]() {
			if (base)
				Interface::ExecuteCommand(Func::SetActorValue, {actor->GetReferenceParam(), RawParameter(index), RawParameter(value)}, key);
			else
				Interface::ExecuteCommand(Func::ForceActorValue, {actor->GetReferenceParam(), RawParameter(index), RawParameter(value)}, key);
		});
	}, key);
}

void Game::DamageActorValue(const FactoryActor& reference, unsigned char index, float value, unsigned int key)
{
	auto* actor = reference.operator->();

	DelayOrExecute(reference, [actor, index, value](unsigned int key) {
		Interface::Dynamic([actor, index, value, key]() {
			Interface::ExecuteCommand(Func::DamageActorValue, {actor->GetReferenceParam(), RawParameter(index), RawParameter(value)}, key);
		});
	}, key);
}

void Game::RestoreActorValue(const FactoryActor& reference, unsigned char index, float value, unsigned int key)
{
	auto* actor = reference.operator->();

	DelayOrExecute(reference, [actor, index, value](unsigned int key) {
		Interface::Dynamic([actor, index, value, key]() {
			Interface::ExecuteCommand(Func::RestoreActorValue, {actor->GetReferenceParam(), RawParameter(index), RawParameter(value)}, key);
		});
	}, key);
}

void Game::SetActorSneaking(const FactoryActor& reference, unsigned int key)
{
	auto* actor = reference.operator->();
	bool sneaking = actor->GetActorSneaking();

	DelayOrExecute(reference, [actor, sneaking](unsigned int key) {
		Interface::Dynamic([actor, sneaking, key]() {
			Interface::ExecuteCommand(Func::SetForceSneak, {actor->GetReferenceParam(), RawParameter(sneaking)}, key);
		});
	}, key);
}

void Game::SetActorAlerted(const FactoryActor& reference, unsigned int key)
{
	auto* actor = reference.operator->();
	bool alerted = actor->GetActorAlerted();

	DelayOrExecute(reference, [actor, alerted](unsigned int key) {
		Interface::Dynamic([actor, alerted, key]() {
			Interface::ExecuteCommand(Func::SetAlert, {actor->GetReferenceParam(), RawParameter(alerted)}, key);
		});
	}, key);
}

void Game::SetActorAnimation(const FactoryActor& reference, unsigned char anim, unsigned int key)
{
	Interface::Dynamic([&reference, anim, key]() {
		Interface::ExecuteCommand(Func::PlayGroup, {reference->GetReferenceParam(), RawParameter(anim), RawParameter(true)}, key);
	});
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
	Interface::Dynamic([&reference, &anim, key]() {
		Interface::ExecuteCommand(Func::PlayIdle, {reference->GetReferenceParam(), RawParameter(anim)}, key);
	});
}

void Game::SetActorRace(const FactoryActor& reference, signed int delta_age, unsigned int key)
{
	auto* actor = reference.operator->();
	unsigned int race = actor->GetActorRace();

	DelayOrExecute(reference, [actor, race, delta_age](unsigned int key) {
		unsigned int baseID = actor->GetBase();

		// set only once per base
		if (baseRaces[baseID] == race || race == UINT_MAX) // creature test
		{
			if (key)
				Lockable::Retrieve(key);
			return;
		}

		baseRaces[baseID] = race;

		Interface::Dynamic([actor, race, delta_age, key]() {
			Interface::ExecuteCommand(Func::MatchRace, {actor->GetReferenceParam(), RawParameter(race)}, delta_age ? 0 : key);

			if (delta_age)
				Interface::ExecuteCommand(Func::AgeRace, {actor->GetReferenceParam(), RawParameter(delta_age)}, key);
		});
	}, key);
}

void Game::SetActorFemale(const FactoryActor& reference, unsigned int key)
{
	auto* actor = reference.operator->();
	bool female = actor->GetActorFemale();

	DelayOrExecute(reference, [actor, female](unsigned int key) {
		if (actor->GetActorRace() == UINT_MAX) // creature test
		{
			if (key)
				Lockable::Retrieve(key);
			return;
		}

		Interface::Dynamic([actor, female, key]() {
			Interface::ExecuteCommand(Func::SexChange, {actor->GetReferenceParam(), RawParameter(female)}, key);
		});
	}, key);
}

void Game::KillActor(const FactoryActor& reference, unsigned int key)
{
	auto* actor = reference.operator->();
	unsigned short limbs = reference->GetActorLimbs();
	signed char cause = reference->GetActorDeathCause();

	DelayOrExecute(reference, [actor, limbs, cause](unsigned int key) {
		Interface::Dynamic([actor, limbs, cause, key]() {
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
		});
	}, key);
}

void Game::FireWeapon(const FactoryActor& reference, unsigned int weapon, unsigned int key)
{
	Interface::Dynamic([&reference, weapon, key]() {
		Interface::ExecuteCommand(Func::FireWeapon, {reference->GetReferenceParam(), RawParameter(weapon)}, key);
	});
}

void Game::AddItem(const FactoryContainer& reference, const FactoryItem& item, unsigned int key)
{
	AddItem(reference, item->GetBase(), item->GetItemCount(), item->GetItemCondition(), item->GetItemSilent(), key);
}

void Game::AddItem(const FactoryContainer& reference, unsigned int baseID, unsigned int count, float condition, bool silent, unsigned int key)
{
	auto* container = reference.operator->();

	DelayOrExecute(reference, [container, baseID, count, condition, silent](unsigned int key) {
		Interface::Dynamic([container, baseID, count, condition, silent, key]() {
			Interface::ExecuteCommand(Func::AddItemHealthPercent, {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(condition / 100), RawParameter(silent)}, key);
		});
	}, key);
}

void Game::RemoveItem(const FactoryContainer& reference, const FactoryItem& item, unsigned int key)
{
	RemoveItem(reference, item->GetBase(), item->GetItemCount(), item->GetItemSilent(), key);
}

void Game::RemoveItem(const FactoryContainer& reference, unsigned int baseID, unsigned int count, bool silent, unsigned int key)
{
	auto* container = reference.operator->();

	DelayOrExecute(reference, [container, baseID, count, silent](unsigned int key) {
		Interface::Dynamic([container, baseID, count, silent, key]() {
			Interface::ExecuteCommand(Func::RemoveItem, {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(silent)}, key);
		});
	}, key);
}

void Game::SetRefCount(const FactoryItem& reference, unsigned int key)
{
	auto* item = reference.operator->();
	unsigned int count = item->GetItemCount();

	DelayOrExecute(reference, [item, count](unsigned int key) {
		Interface::Dynamic([item, count, key]() {
			Interface::ExecuteCommand(Func::SetRefCount, {item->GetReferenceParam(), RawParameter(count)}, key);
		});
	}, key);
}

void Game::SetCurrentHealth(const FactoryItem& reference, unsigned int health, unsigned int key)
{
	auto* item = reference.operator->();

	DelayOrExecute(reference, [item, health](unsigned int key) {
		Interface::Dynamic([item, health, key]() {
			Interface::ExecuteCommand(Func::SetCurrentHealth, {item->GetReferenceParam(), RawParameter(health)}, key);
		});
	}, key);
}

void Game::EquipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key)
{
	EquipItem(reference, item->GetBase(), item->GetItemCondition(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::EquipItem(const FactoryActor& reference, unsigned int baseID, float condition, bool silent, bool stick, unsigned int key)
{
	auto* actor = reference.operator->();

	DelayOrExecute(reference, [actor, baseID, condition, silent, stick](unsigned int key) {
		Interface::Dynamic([actor, baseID, condition, silent, stick, key]() {
			Interface::ExecuteCommand(Func::EquipItem, {actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);
		});
		// Add: adjust condition
	}, key);
}

void Game::UnequipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key)
{
	UnequipItem(reference, item->GetBase(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::UnequipItem(const FactoryActor& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	auto* actor = reference.operator->();

	DelayOrExecute(reference, [actor, baseID, silent, stick](unsigned int key) {
		Interface::Dynamic([actor, baseID, silent, stick, key]() {
			Interface::ExecuteCommand(Func::UnequipItem, {actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);
		});
	}, key);
}

void Game::SetWindowPos(const FactoryWindow& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUIPos, {RawParameter(reference->GetLabel()), RawParameter(get<0>(reference->GetPos())), RawParameter(get<1>(reference->GetPos())), RawParameter(get<2>(reference->GetPos())), RawParameter(get<3>(reference->GetPos()))});
	});
}

void Game::SetWindowSize(const FactoryWindow& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUISize, {RawParameter(reference->GetLabel()), RawParameter(get<0>(reference->GetSize())), RawParameter(get<1>(reference->GetSize())), RawParameter(get<2>(reference->GetSize())), RawParameter(get<3>(reference->GetSize()))});
	});
}

void Game::SetWindowVisible(const FactoryWindow& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUIVisible, {RawParameter(reference->GetLabel()), RawParameter(reference->GetVisible())});
	});
}

void Game::SetWindowLocked(const FactoryWindow& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUILocked, {RawParameter(reference->GetLabel()), RawParameter(reference->GetLocked())});
	});
}

void Game::SetWindowText(const FactoryWindow& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUIText, {RawParameter(reference->GetLabel()), RawParameter(reference->GetText())});
	});
}

void Game::SetEditMaxLength(const FactoryEdit& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUIMaxLen, {RawParameter(reference->GetLabel()), RawParameter(reference->GetMaxLength())});
	});
}

void Game::SetEditValidation(const FactoryEdit& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUIValid, {RawParameter(reference->GetLabel()), RawParameter(reference->GetValidation())});
	});
}

void Game::SetCheckboxSelected(const FactoryCheckbox& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUICheckbox, {RawParameter(reference->GetLabel()), RawParameter(reference->GetSelected())});
	});
}

void Game::SetRadioButtonSelected(const FactoryRadioButton& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUICheckbox, {RawParameter(reference->GetLabel()), RawParameter(reference->GetSelected())});
	});
}

void Game::SetRadioButtonGroup(const FactoryRadioButton& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUIRadioGroup, {RawParameter(reference->GetLabel()), RawParameter(reference->GetGroup())});
	});
}

void Game::SetListItemSelected(const FactoryListItem& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUISelectChange, {RawParameter(reference->GetItemContainer()), RawParameter(reference->GetNetworkID()), RawParameter(reference->GetSelected())});
	});
}

void Game::SetListItemText(const FactoryListItem& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUISelectText, {RawParameter(reference->GetItemContainer()), RawParameter(reference->GetNetworkID()), RawParameter(reference->GetText())});
	});
}

void Game::SetListMultiSelect(const FactoryList& reference)
{
	Interface::Dynamic([&reference]() {
		Interface::ExecuteCommand(Func::GUISelectMulti, {RawParameter(reference->GetLabel()), RawParameter(reference->GetMultiSelect())});
	});
}

void Game::SetWindowMode()
{
	Interface::Dynamic([]() {
		if (GUIMode)
		{
			DisablePlayerControls(true, true, true, false);
			ToggleControl(false, ControlCode_TogglePOV);
			ToggleControl(false, ControlCode_Crouch);
		}
		else
		{
			EnablePlayerControls();
			ToggleControl(true, ControlCode_TogglePOV);
			ToggleControl(true, ControlCode_Crouch);
		}

		Interface::ExecuteCommand(Func::GUIMode, {RawParameter(GUIMode)});
	});
}

void Game::EnablePlayerControls(bool movement, bool pipboy, bool fighting, bool pov, bool looking, bool rollover, bool sneaking)
{
	Interface::Dynamic([movement, pipboy, fighting, pov, looking, rollover, sneaking]() {
		Interface::ExecuteCommand(Func::EnablePlayerControls, {RawParameter(movement), RawParameter(pipboy), RawParameter(fighting), RawParameter(pov), RawParameter(looking), RawParameter(rollover), RawParameter(sneaking)});
	});
}

void Game::DisablePlayerControls(bool movement, bool pipboy, bool fighting, bool pov, bool looking, bool rollover, bool sneaking)
{
	Interface::Dynamic([movement, pipboy, fighting, pov, looking, rollover, sneaking]() {
		Interface::ExecuteCommand(Func::DisablePlayerControls, {RawParameter(movement), RawParameter(pipboy), RawParameter(fighting), RawParameter(pov), RawParameter(looking), RawParameter(rollover), RawParameter(sneaking)});
	});
}

void Game::ToggleKey(bool enabled, unsigned int scancode)
{
	Interface::Dynamic([enabled, scancode]() {
		if (enabled)
			Interface::ExecuteCommand(Func::EnableKey, {RawParameter(scancode)});
		else
			Interface::ExecuteCommand(Func::DisableKey, {RawParameter(scancode)});
	});
}

void Game::ToggleControl(bool enabled, unsigned int control)
{
	Interface::Dynamic([enabled, control]() {
		if (enabled)
			Interface::ExecuteCommand(Func::EnableControl, {RawParameter(control)});
		else
			Interface::ExecuteCommand(Func::DisableControl, {RawParameter(control)});
	});
}

void Game::SetWeather(unsigned int weather)
{
	Interface::Dynamic([weather]() {
		Interface::ExecuteCommand(Func::ForceWeather, {RawParameter(weather), RawParameter(1)});
	});
}

void Game::ForceRespawn()
{
	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::Dynamic([key]() {
		Interface::ExecuteCommand(Func::ForceRespawn, {}, key);
	});

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

	return cellContext.Operate([cell](Player::CellContext& cellContext) {
		return find(cellContext.begin(), cellContext.end(), cell) != cellContext.end();
	});
}

vector<unsigned int> Game::GetContext(unsigned int type)
{
	return cellRefs.Operate([type](CellRefs& cellRefs) {
		return cellContext.Operate([&cellRefs, type](Player::CellContext& cellContext) {
			vector<unsigned int> result;

			for (unsigned int cell : cellContext)
				if (cell)
					for (const auto& refs : cellRefs[cell])
						if (refs.first & type)
							result.insert(result.end(), refs.second.begin(), refs.second.end());

			return result;
		});
	});
}

unsigned int Game::GetAnchor(unsigned int cell)
{
	return cellRefs.Operate([cell](CellRefs& cellRefs) {
		const auto& refs = cellRefs[cell];

		unsigned int anchorID = [&refs]() {
			for (const auto& subrefs : refs)
				if (!subrefs.second.empty())
					return *subrefs.second.begin();

			return 0x00000000u;
		}();

		if (!anchorID)
			throw VaultException("No anchor reference in cell %08X", cell).stacktrace();

		return anchorID;
	});
}

void Game::net_SetName(const FactoryObject& reference, const string& name)
{
	bool result = static_cast<bool>(reference->SetName(name));

	if (result)
		SetName(reference);
}

void Game::net_SetPos(const FactoryObject& reference, float X, float Y, float Z)
{
	bool result = static_cast<bool>(reference->SetNetworkPos(tuple<float, float, float>{X, Y, Z}));

	if (result && reference->GetEnabled())
	{
		auto actor = vaultcast<Actor>(reference); // maybe we should consider items, too (they have physics)

		//if (!actor || (!reference->IsNearPoint(reference->GetNetworkPos(Axis_X), reference->GetNetworkPos(Axis_Y), reference->GetNetworkPos(Axis_Z), 50.0)) || actor->IsActorJumping() || actor->GetReference() == PLAYER_REFERENCE)

		SetPos(reference);
	}
}

void Game::net_SetAngle(const FactoryObject& reference, float X, float Y, float Z)
{
	bool result = static_cast<bool>(reference->SetAngle(tuple<float, float, float>{X, Y, Z}));

	if (result && reference->GetEnabled())
	{
		SetAngle(reference);

		auto actor = vaultcast<Actor>(reference);

		if (actor)
		{
			static map<AnimationGroups, array<AnimationGroups, 2>> reactions = {
				{AnimGroup_AimIS, {{AnimGroup_AimISDown, AnimGroup_AimISUp}}},
				{AnimGroup_AttackSpin2, {{AnimGroup_AimUp, AnimGroup_AimDown}}},
			};

			auto anim = static_cast<AnimationGroups>(actor->GetActorWeaponAnimation());

			if (reactions.count(anim))
			{
				SetActorAnimation(actor.get(), reactions[anim][0]);
				SetActorAnimation(actor.get(), reactions[anim][1]);
			}
		}
	}
}

void Game::net_SetCell(FactoryObject& reference, FactoryPlayer& player, unsigned int cell, float X, float Y, float Z)
{
	unsigned int old_cell = reference->GetNetworkCell();
	reference->SetNetworkCell(cell);

	bool result = false;

	if (X || Y || Z)
		result = static_cast<bool>(reference->SetNetworkPos(tuple<float, float, float>{X, Y, Z}));

	if (reference->GetReference())
	{
		if (reference != player)
		{
			if (IsInContext(cell))
			{
				if (IsInContext(old_cell))
				{
					cellRefs.Operate([&reference, old_cell, cell](CellRefs& cellRefs) {
						cellRefs[old_cell][reference.GetType()].erase(reference->GetReference());
						cellRefs[cell][reference.GetType()].insert(reference->GetReference());
					});

					if (reference->SetEnabled(true))
						ToggleEnabled(reference);

					if (reference->SetGameCell(cell))
						MoveTo(reference, player, true);
				}
				else
				{
					cellRefs.Operate([&reference, old_cell](CellRefs& cellRefs) {
						cellRefs[old_cell][reference.GetType()].erase(reference->GetReference());
					});

					RemoveObject(reference);
					reference->SetReference(0x00000000);
					reference->SetEnabled(false);

					GameFactory::Free(player);
					NewDispatch(reference);
				}
			}
			else
			{
				cellRefs.Operate([&reference, old_cell](CellRefs& cellRefs) {
					cellRefs[old_cell][reference.GetType()].erase(reference->GetReference());
				});

				uninitObj.Operate([&reference, cell](UninitializedObjects& uninitObj) {
					uninitObj[cell].push_back(reference->GetNetworkID());
				});

				RemoveObject(reference);
				reference->SetReference(0x00000000);
				reference->SetEnabled(false);
			}
		}
		else if (result)
			SetPos(player);
	}
	else
	{
		bool context = IsInContext(cell);

		uninitObj.Operate([&reference, old_cell, cell, context](UninitializedObjects& uninitObj) {
			NetworkID id = reference->GetNetworkID();

			auto& sub = uninitObj[old_cell];
			sub.erase(find(sub.begin(), sub.end(), id));

			if (!context)
				uninitObj[cell].push_back(id);
		});

		if (context)
		{
			GameFactory::Free(player);
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

void Game::net_GetActivate(const FactoryReference& reference, const FactoryReference& actor)
{
	Activate(reference, actor);
}

void Game::net_PlaySound(const FactoryObject& reference, unsigned int sound)
{
	PlaySound(reference, sound);
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
			float condition = reference->GetItemCondition();

			GameFactory::Free(reference);

			GameFactory::Operate<Container, EXCEPTION_FACTORY_VALIDATED>(container, [count, old_count, baseID, condition, silent](FactoryContainer& container) {
				signed int diff = count - old_count;

				if (diff > 0)
					AddItem(container, baseID, diff, condition, silent);
				else
				{
					RemoveItem(container, baseID, abs(diff), silent);

					// Game always removes equipped item first - workaround (is this really always the case?)
					NetworkID equipped = container->IsEquipped(baseID);

					if (equipped && vaultcast_test<Actor>(container))
						GameFactory::Operate<Actor, EXCEPTION_FACTORY_VALIDATED>(container->GetNetworkID(), [equipped](FactoryActor& actor) {
							GameFactory::Operate<Item>(equipped, [&actor](Item* item) {
								EquipItem(actor, item->GetBase(), item->GetItemCondition(), false, item->GetItemStick());
							});
						});
				}
			});
		}
	}
}

void Game::net_SetItemCondition(FactoryItem& reference, float condition, unsigned int health)
{
	if (reference->SetItemCondition(condition))
	{
		NetworkID container = reference->GetItemContainer();

		if (!container)
			SetCurrentHealth(reference, health);
		else if (reference->GetItemEquipped())
		{
			GameFactory::Free(reference);

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

		GameFactory::Free(reference);

		GameFactory::Operate<Actor, EXCEPTION_FACTORY_VALIDATED>(container, [equipped, item](FactoryActor& actor) {
			GameFactory::Operate<Item, EXCEPTION_FACTORY_VALIDATED>(item, [&actor, equipped](FactoryItem& item) {
				if (equipped)
					EquipItem(actor, item);
				else
					UnequipItem(actor, item);
			});
		});
	}
}

void Game::net_SetActorValue(const FactoryActor& reference, bool base, unsigned char index, float value)
{
	Lockable* result;

	float prev_value = reference->GetActorValue(index);

	if (base)
		result = reference->SetActorBaseValue(index, value);
	else
		result = reference->SetActorValue(index, value);

	if (result)
	{
		if (!base && (index == ActorVal_Health || (index >= ActorVal_Head && index <= ActorVal_Brain)))
		{
			float diff = value - prev_value;

			if (diff < 0.00f)
				DamageActorValue(reference, index, diff);
			else if (diff > 0.00f)
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

	result = reference->SetActorDead(dead, limbs, cause);

	if (result && reference->GetReference())
	{
		if (dead)
			KillActor(reference);
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
			GameFactory::Free(reference);

			ForceRespawn();

			this_thread::sleep_for(chrono::seconds(3));

			// remove all base effects so they get re-applied in LoadEnvironment
			baseRaces.clear();
			spawnFunc();

			cellContext.Operate([](Player::CellContext& cellContext) {
				cellContext = spawnContext;
			});

			LoadEnvironment();

			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, false, 0, 0),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
			});
		}
	}
}

void Game::net_FireWeapon(const FactoryActor& reference, unsigned int weapon)
{
	if (reference->GetEnabled())
		FireWeapon(reference, weapon);
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

	CellRefs copy;
	pair<vector<unsigned int>, vector<unsigned int>> diff;

	GameFactory::Operate<Player>(PLAYER_REFERENCE, [&context, &copy, &diff](Player* player) {
		unsigned int old_cell = player->GetNetworkCell();

		player->SetNetworkCell(context[0]);
		player->SetGameCell(context[0]);

		cellRefs.Operate([&context, &copy, &diff, old_cell](CellRefs& cellRefs) {
			cellContext.Operate([&context, &copy, &diff, old_cell, &cellRefs](Player::CellContext& cellContext) {
				cellRefs[old_cell][ID_PLAYER].erase(PLAYER_REFERENCE);
				cellRefs[context[0]][ID_PLAYER].insert(PLAYER_REFERENCE);

				sort(context.begin(), context.end());
				sort(cellContext.begin(), cellContext.end());

				set_difference(context.begin(), context.end(), cellContext.begin(), cellContext.end(), back_inserter(diff.first));
				set_difference(cellContext.begin(), cellContext.end(), context.begin(), context.end(), back_inserter(diff.second));

				cellContext = context;

				for (auto cell : diff.second)
					if (cell)
						copy[cell] = cellRefs[cell];

				for (auto cell : diff.first)
					if (cell)
						copy[cell] = cellRefs[cell];
			});
		});
	});

	for (unsigned int cell : diff.second)
		if (cell)
			for (const auto& refs : copy[cell])
				for (unsigned int refID : refs.second)
					if (refID != PLAYER_REFERENCE)
						GameFactory::Operate<Object, EXCEPTION_FACTORY_VALIDATED>(refID, [](FactoryObject& object) {
							if (!object->IsPersistent() && object->SetEnabled(false))
								ToggleEnabled(object);
						});

	for (unsigned int cell : diff.first)
		if (cell)
		{
			vector<unsigned int> refIDs;

			deletedObj.Operate([&refIDs, cell](DeletedObjects& deletedObj) {
				refIDs = move(deletedObj[cell]);
			});

			for (auto id : refIDs)
				RemoveObject(id);

			for (const auto& refs : copy[cell])
				for (unsigned int refID : refs.second)
					if (refID != PLAYER_REFERENCE)
						GameFactory::Operate<Object, EXCEPTION_FACTORY_VALIDATED>({refID, PLAYER_REFERENCE}, [cell](FactoryObjects& objects) {
							if (objects[0]->SetEnabled(true))
								ToggleEnabled(objects[0]);

							if (objects[0]->SetGameCell(cell))
								MoveTo(objects[0], objects[1], true);

							objects[0]->Work();
						});

			UninitializedObjects::mapped_type ids;

			uninitObj.Operate([&ids, cell](UninitializedObjects& uninitObj) {
				ids = move(uninitObj[cell]);
			});

			for (auto id : ids)
				GameFactory::Operate<Object, EXCEPTION_FACTORY_VALIDATED>(id, [](FactoryObject& object) {
					NewDispatch(object);
				});
		}
}

void Game::net_UpdateConsole(bool enabled)
{
	ToggleKey(enabled, ScanCode_Console);
}

void Game::net_UpdateWindowPos(const FactoryWindow& reference, const tuple<float, float, float, float>& pos)
{
	reference->SetPos(get<0>(pos), get<1>(pos), get<2>(pos), get<3>(pos));

	SetWindowPos(reference);
}

void Game::net_UpdateWindowSize(const FactoryWindow& reference, const tuple<float, float, float, float>& size)
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

void Game::net_UpdateCheckboxSelected(const FactoryCheckbox& reference, bool selected)
{
	reference->SetSelected(selected);

	SetCheckboxSelected(reference);
}

void Game::net_UpdateRadioButtonSelected(const FactoryRadioButton& reference, ExpectedRadioButton& previous, bool selected)
{
	if (previous)
		previous->SetSelected(false);

	reference->SetSelected(selected);

	SetRadioButtonSelected(reference);
}

void Game::net_UpdateRadioButtonGroup(const FactoryRadioButton& reference, unsigned int group)
{
	reference->SetGroup(group);

	SetRadioButtonGroup(reference);
}

void Game::net_UpdateListItemSelected(const FactoryListItem& reference, bool selected)
{
	reference->SetSelected(selected);

	SetListItemSelected(reference);
}

void Game::net_UpdateListItemText(const FactoryListItem& reference, const string& text)
{
	reference->SetText(text);

	SetListItemText(reference);
}

void Game::net_UpdateListMultiSelect(const FactoryList& reference, bool multiselect)
{
	reference->SetMultiSelect(multiselect);

	SetListMultiSelect(reference);
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
	globals[global] = value;

	SetGlobalValue(global, value);
}

void Game::net_SetWeather(unsigned int weather)
{
	Game::weather = weather;

	SetWeather(weather);
}

void Game::net_SetBase(unsigned int playerBase)
{
	Game::playerBase = playerBase;
}

void Game::net_SetDeletedStatic(DeletedObjects&& deletedStatic)
{
	Game::deletedStatic.Operate([deletedStatic = move(deletedStatic)](DeletedObjects& deletedStatic_) {
		deletedStatic_ = move(deletedStatic);
	});
}

void Game::GetPos(const FactoryObject& reference, float X, float Y, float Z)
{
	bool result = static_cast<bool>(reference->SetGamePos(tuple<float, float, float>{X, Y, Z}));

	if (result && reference->GetReference() == PLAYER_REFERENCE)
	{
		reference->SetNetworkPos(tuple<float, float, float>{X, Y, Z});

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_POS>(reference->GetNetworkID(), X, Y, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
	}
}

void Game::GetAngle(const FactoryObject& reference, float X, float Y, float Z)
{
	if (reference->GetReference() != PLAYER_REFERENCE)
		return;

	bool result = static_cast<bool>(reference->SetAngle(tuple<float, float, float>{X, Y, Z}));

	if (result)
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(reference->GetNetworkID(), X, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
}

void Game::GetParentCell(const FactoryPlayer& player, unsigned int cell)
{
	bool result = static_cast<bool>(player->SetGameCell(cell));

	if (result)
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(player->GetNetworkID(), cell, 0.0f, 0.0f, 0.0f),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
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
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(reference->GetNetworkID(), idle, moving, movingxy, reference->GetActorWeaponAnimation(), reference->GetActorAlerted(), sneaking, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
}

void Game::GetControl(const FactoryPlayer& reference, unsigned char control, unsigned char key)
{
	bool result = static_cast<bool>(reference->SetPlayerControl(control, key));

	if (result)
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_CONTROL>(reference->GetNetworkID(), control, key),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
}

void Game::GetActivate(const FactoryReference& reference, const FactoryReference& actor)
{
	Network::Queue({{
		PacketFactory::Create<pTypes::ID_UPDATE_ACTIVATE>(reference->GetNetworkID(), actor->GetNetworkID()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
	});
}

void Game::GetFireWeapon(const FactoryPlayer& reference)
{
	Network::Queue({{
		PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(reference->GetNetworkID(), 0),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
	});
}

void Game::GetMessage(string message)
{
	if (message.empty())
		return;

	if (message.length() > MAX_CHAT_LENGTH)
		message.resize(MAX_CHAT_LENGTH);

	Network::Queue({{
		PacketFactory::Create<pTypes::ID_GAME_CHAT>(message),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
	});
}

void Game::GetWindowMode(bool enabled)
{
	GUIMode = enabled;

	Interface::Dynamic([]() {
		if (GUIMode)
		{
			DisablePlayerControls(true, true, true, false);
			ToggleControl(false, ControlCode_TogglePOV);
			ToggleControl(false, ControlCode_Crouch);
		}
		else
		{
			EnablePlayerControls();
			ToggleControl(true, ControlCode_TogglePOV);
			ToggleControl(true, ControlCode_Crouch);
		}
	});

	Network::Queue({{
		PacketFactory::Create<pTypes::ID_UPDATE_WMODE>(enabled),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
	});
}

void Game::GetWindowClick(const string& name)
{
	if (!name.compare(Button::CLOSE_BUTTON))
		Interface::SignalEnd();
	else
	{
		NetworkID window = strtoull(name.c_str(), nullptr, 10);

		GameFactory::Operate<Window>(window, [](Window* window) {
			Network::Queue({{
				PacketFactory::Create<pTypes::ID_UPDATE_WCLICK>(window->GetNetworkID()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
			});
		});
	}
}

void Game::GetWindowReturn(const string& name)
{
	NetworkID window = strtoull(name.c_str(), nullptr, 10);

	if (!window)
		return;

	GameFactory::Operate<Window>(window, [](Window* window) {
		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WRETURN>(window->GetNetworkID()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
	});
}

void Game::GetWindowText(const string& name, const string& text)
{
	NetworkID window = strtoull(name.c_str(), nullptr, 10);

	GameFactory::Operate<Window>(window, [&text](Window* window) {
		window->SetText(text);

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WTEXT>(window->GetNetworkID(), text),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
	});
}

void Game::GetCheckboxSelected(const string& name, bool selected)
{
	NetworkID checkbox = strtoull(name.c_str(), nullptr, 10);

	if (!GameFactory::Operate<Checkbox, BOOL_VALIDATED>(checkbox, [selected](Checkbox* checkbox) {
		checkbox->SetSelected(selected);

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WSELECTED>(checkbox->GetNetworkID(), selected),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
	}))
	{
		if (!selected)
			throw VaultException("Radio button event should only fire on selection");

		unsigned int group = GameFactory::Operate<RadioButton>(checkbox, [](RadioButton* radiobutton) {
			radiobutton->SetSelected(true);
			return radiobutton->GetGroup();
		});

		NetworkID previous = GameFactory::Operate<RadioButton>(GameFactory::GetByType(ID_RADIOBUTTON), [checkbox, group](RadioButtons& radiobuttons) {
			for (auto radiobutton : radiobuttons)
				if (radiobutton->GetGroup() == group && radiobutton->GetSelected() && radiobutton->GetNetworkID() != checkbox)
				{
					radiobutton->SetSelected(false);
					return radiobutton->GetNetworkID();
				}

			return 0ull;
		});

		Network::Queue({{
			PacketFactory::Create<pTypes::ID_UPDATE_WRSELECTED>(checkbox, previous, true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server}
		});
	}
}

void Game::GetListboxSelections(const string& name, const vector<const char*>& selections)
{
	NetworkID list = strtoull(name.c_str(), nullptr, 10);

	set<NetworkID> selected_ids;
	transform(selections.begin(), selections.end(), inserter(selected_ids, selected_ids.begin()), [](const char* selection) { return strtoull(selection, nullptr, 10); });

	NetworkResponse r_deselected, r_selected;

	GameFactory::Operate<List>(list, [&selected_ids, &r_deselected, &r_selected](List* list) {
		GameFactory::Operate<ListItem>(list->GetItemList(), [&selected_ids, &r_deselected, &r_selected](ListItems& listitems) {
			for (auto listitem : listitems)
			{
				NetworkID id = listitem->GetNetworkID();
				bool old_selected = listitem->GetSelected();
				bool selected = selected_ids.count(id);

				if (selected == old_selected)
					continue;

				if (selected && !old_selected)
					r_selected.emplace_back(
						PacketFactory::Create<pTypes::ID_UPDATE_WLSELECTED>(id, true),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server);
				else
					r_deselected.emplace_back(
						PacketFactory::Create<pTypes::ID_UPDATE_WLSELECTED>(id, false),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server);

				listitem->SetSelected(selected);
			}
		});
	});

	if (!r_deselected.empty())
		Network::Queue(move(r_deselected));

	if (!r_selected.empty())
		Network::Queue(move(r_selected));
}
