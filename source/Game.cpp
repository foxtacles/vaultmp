#include "Game.h"
#include "PacketFactory.h"
#include "GameFactory.h"

using namespace std;
using namespace RakNet;
using namespace Values;

unsigned char Game::game = 0x00;
RakNetGUID Game::server;

Guarded<Game::CellRefs> Game::cellRefs;
Game::BaseRaces Game::baseRaces;
Game::Globals Game::globals;
Game::Weather Game::weather;

function<void()> Game::spawnFunc;

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
		//debug->PrintFormat("Executing command %04hX on reference %08X, key %08X", true, opcode, info.size() > 1 ? getFrom<unsigned int>(info.at(1)) : 0, key);
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
				case Fallout3::Func_GetRefCount:
				case FalloutNV::Func_GetRefCount:
				case Fallout3::Func_GetBaseObject:
				case FalloutNV::Func_GetBaseObject:
				case Fallout3::Func_CenterOnWorld:
				case FalloutNV::Func_CenterOnWorld:
				case Fallout3::Func_Load:
				case FalloutNV::Func_Load:
					shared = Lockable::Poll(key);
					break;

				case Func_IsLimbGone:
				case Fallout3::Func_GetFirstRef:
				case FalloutNV::Func_GetFirstRef:
				case Fallout3::Func_GetNextRef:
				case FalloutNV::Func_GetNextRef:
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
				Game::GetActorValue(reference.get(), false, getFrom<unsigned char>(info.at(2)), result);
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

			case Func_Chat:
			{
				if (!result)
					break;

				vector<unsigned char>& data = *getFrom<vector<unsigned char>*>(result);
				GetMessage(string(reinterpret_cast<char*>(&data[0]), data.size()));
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

			case Fallout3::Func_GetBaseObject:
			case FalloutNV::Func_GetBaseObject:
				FutureSet(shared, getFrom<unsigned int>(result));
				break;

			case Fallout3::Func_GetRefCount:
			case FalloutNV::Func_GetRefCount:
				FutureSet(shared, static_cast<unsigned int>(result));
				break;

			case Fallout3::Func_SetRefCount:
			case FalloutNV::Func_SetRefCount:
				break;

			case Fallout3::Func_GetFirstRef:
			case FalloutNV::Func_GetFirstRef:
				GetNextRef(key, getFrom<unsigned int>(result), getFrom<unsigned int>(info.at(1)));
				break;

			case Fallout3::Func_GetNextRef:
			case FalloutNV::Func_GetNextRef:
				GetNextRef(key, getFrom<unsigned int>(result));
				break;

			case Func_UIMessage:
				break;

			case Fallout3::Func_GetParentCell:
			case FalloutNV::Func_GetParentCell:
			{
				auto objects = GameFactory::GetMultiple<Actor>(vector<unsigned int>{getFrom<unsigned int>(info.at(1)), PLAYER_REFERENCE});
				GetParentCell(objects[0].get(), vaultcast<Player>(objects[1]).get(), getFrom<unsigned int>(result));
				break;
			}

			case Fallout3::Func_EnableControl:
			case FalloutNV::Func_EnableControl:
				break;

			case FalloutNV::Func_DisableControl:
				break;

			case FalloutNV::Func_GetControl:
				if (game == FALLOUT3) // Fallout3::Func_DisableControl
					break;
			case Fallout3::Func_GetControl:
			{
				auto self = GameFactory::GetObject<Player>(PLAYER_REFERENCE);
				GetControl(self.get(), getFrom<int>(info.at(1)), result);
				break;
			}

			case Fallout3::Func_DisableKey:
			case FalloutNV::Func_DisableKey:
				break;

			case Fallout3::Func_EnableKey:
			case FalloutNV::Func_EnableKey:
				break;

			case Func_CenterOnCell:
			case Func_CenterOnExterior:
			case Func_ForceRespawn:
			case Fallout3::Func_CenterOnWorld:
			case FalloutNV::Func_CenterOnWorld:
			case Fallout3::Func_Load:
			case FalloutNV::Func_Load:
				FutureSet(shared, true);
				break;

			case Fallout3::Func_SetName:
			case FalloutNV::Func_SetName:
				break;

			default:
				throw VaultException("Unhandled function %04hX", opcode);
		}
	}
	else
	{
#ifdef VAULTMP_DEBUG
		debug.print("Command ", hex, opcode, " failed");
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

	return NetworkResponse{Network::CreateResponse(
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
		ScanCode_Escape})});

	Interface::EndDynamic();

	Interface::StartSetup();

	Interface::SetupCommand("GetPos", {self_ref, Object::Param_Axis()});
	Interface::SetupCommand("GetPos", {Player::CreateFunctor(FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE), Object::Param_Axis()}, 30);
	Interface::SetupCommand("GetAngle", {self_ref, RawParameter(vector<string> {API::RetrieveAxis_Reverse(Axis_X), API::RetrieveAxis_Reverse(Axis_Z)})});
	Interface::SetupCommand("GetActorState", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Player::CreateFunctor(FLAG_MOVCONTROLS, id)});
	Interface::SetupCommand("GetParentCell", {Player::CreateFunctor(FLAG_ALIVE)}, 30);
	Interface::SetupCommand("ScanContainer", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED)}, 50);
	Interface::SetupCommand("GetDead", {Player::CreateFunctor(FLAG_ENABLED | FLAG_ALIVE)}, 30);

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
	Interface::SetupCommand("GetActorValue", {Player::CreateFunctor(FLAG_NOTSELF | FLAG_SELFALERT | FLAG_ENABLED | FLAG_ALIVE), health}, 30);

	// we could exclude health values here
	Interface::SetupCommand("GetActorValue", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Actor::Param_ActorValues()}, 100);
	Interface::SetupCommand("GetBaseActorValue", {Player::CreateFunctor(FLAG_SELF | FLAG_ENABLED), Actor::Param_ActorValues()}, 200);

	Interface::EndSetup();
}

template <typename T>
void Game::FutureSet(const weak_ptr<Lockable>& data, T&& t)
{
	shared_ptr<Lockable> shared = data.lock();
	Lockable* locked = shared.get();

	if (locked == nullptr)
		throw VaultException("Storage has expired");

	Shared<T>* store = dynamic_cast<Shared<T>*>(locked);

	if (store == nullptr)
		throw VaultException("Storage is corrupted");

	(**store) = forward<T>(t);
	store->set_promise();
}

void Game::AsyncDispatch(function<void()>&& func)
{
	thread t(move(func));
	t.detach();
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
		throw VaultException("Loading of savegame %s failed (%s)", savegame.c_str(), e.what());
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
		throw VaultException("Loading of cell %s failed (%s)", cell.c_str(), e.what());
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
		throw VaultException("Loading of cell (%d,%d) failed (%s)", x, y, e.what());
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
		throw VaultException("Loading of world (%08X,%d,%d) failed (%s)", baseID, x, y, e.what());
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

	for (NetworkID& id : reference)
	{
		auto _reference = GameFactory::GetObject(id);
		auto& reference = _reference.get();

		if (!reference->IsPersistent())
		{
			cellRefs.StartSession();
			(*cellRefs)[reference->GetNetworkCell()][FormType_Inventory].erase(reference->GetReference());
			cellRefs.EndSession();

			reference->SetReference(0x00000000);
		}

		unsigned char type = reference.GetType();

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
				throw VaultException("Can't create object of unknown type %02X", type);
		}
	}
}

void Game::UIMessage(const string& message)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("UIMessage", {RawParameter(message)});

	Interface::EndDynamic();
}

void Game::ChatMessage(const string& message)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("ChatMessage", {RawParameter(message)});

	Interface::EndDynamic();
}

void Game::NewObject(FactoryObject<Object>& reference)
{
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
			throw VaultException("Object creation with baseID %08X and NetworkID %llu failed (%s)", baseID, id, e.what());
		}

		reference = GameFactory::GetObject(id).get();
		reference->SetReference(refID);
	}
	//else if (!(not in player cell) || !reference->GetChanged())
		//return;

	unsigned int refID = reference->GetReference();

	SetName(reference);
	SetAngle(reference);

	// experimental
	if (refID != PLAYER_REFERENCE)
	{
		AsyncDispatch([=]
		{
			try
			{
				this_thread::sleep_for(chrono::milliseconds(500));

				auto objects = GameFactory::GetMultiple(vector<unsigned int>{refID, PLAYER_REFERENCE});

				auto& object = objects[0].get();
				auto& player = objects[1].get();

				unsigned int cell = player->GetGameCell();

				if (reference->GetNetworkCell() == cell)
				{
					MoveTo(object, player, true);
					reference->SetEnabled(true);
				}
				else
				{
					reference->SetEnabled(false);
					ToggleEnabled(object);
				}

				reference->SetGameCell(cell);
			}
			catch (...) {}
		});
	}

	// maybe more
}

void Game::NewItem(FactoryObject<Item>& reference)
{
	NetworkID id = reference->GetItemContainer();

	if (id)
		throw VaultException("Cannot create item %llu which is bound to a Container (%llu)", reference->GetNetworkID(), id);

	NewObject(reference);
	SetRefCount(reference);

	cellRefs.StartSession();
	(*cellRefs)[reference->GetNetworkCell()][FormType_Inventory].insert(reference->GetReference());
	cellRefs.EndSession();
}

void Game::NewContainer(FactoryObject<Container>& reference)
{
	NewObject(reference);
	auto items = GameFactory::GetMultiple<Item>(vector<NetworkID>(reference->GetItemList().begin(), reference->GetItemList().end()));

	for (auto& _item : items)
	{
		AddItem(reference, _item.get());
		//debug->PrintFormat("ID: %llu, %s, %08X, %d, %d, %d, %d", true, item->GetNetworkID(), item->GetName().c_str(), item->GetBase(), (int)item->GetItemEquipped(), (int)item->GetItemSilent(), (int)item->GetItemStick(), item->GetItemCount());

		if (_item->GetItemEquipped())
			EquipItem(vaultcast<Actor>(reference).get(), _item.get());
	}
}

void Game::NewActor(FactoryObject<Actor>& reference)
{
	NewContainer(reference);

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

		if (reference->GetActorAlerted())
			SetActorAlerted(reference)();

		if (reference->GetActorSneaking())
			SetActorSneaking(reference)();

		if (reference->GetActorMovingAnimation() != AnimGroup_Idle)
			SetActorMovingAnimation(reference);

		if (reference->GetActorWeaponAnimation() != AnimGroup_Idle)
			SetActorWeaponAnimation(reference);
	}
}

void Game::NewPlayer(FactoryObject<Player>& reference)
{
	NewActor(reference);

	// ...
}

void Game::RemoveObject(const FactoryObject<Object>& reference)
{
	if (reference->SetEnabled(false))
		ToggleEnabled(reference);

	Interface::StartDynamic();

	Interface::ExecuteCommand("MarkForDelete", {reference->GetReferenceParam()});

	Interface::EndDynamic();

	cellRefs.StartSession();
	(*cellRefs)[reference->GetNetworkCell()][FormType_Inventory].erase(reference->GetReference());
	cellRefs.EndSession();
}

void Game::PlaceAtMe(const FactoryObject<Object>& reference, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	PlaceAtMe(reference->GetReference(), baseID, condition, count, key);
}

void Game::PlaceAtMe(unsigned int refID, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("PlaceAtMeHealthPercent", {RawParameter(refID), RawParameter(baseID), RawParameter(condition), RawParameter(count)}, key);

	Interface::EndDynamic();
}

void Game::ToggleEnabled(const FactoryObject<Object>& reference)
{
	Interface::StartDynamic();

	if (reference->GetEnabled())
		Interface::ExecuteCommand("Enable", {reference->GetReferenceParam(), RawParameter(true)});
	else
		Interface::ExecuteCommand("Disable", {reference->GetReferenceParam(), RawParameter(false)});

	Interface::EndDynamic();
}

void Game::Delete(FactoryObject<Object>& reference)
{
	RemoveObject(reference);
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
		throw VaultException("Obtaining of baseID of refID %08X (%s)", refID, e.what());
	}

	return baseID;
}

void Game::SetName(const FactoryObject<Object>& reference)
{
	string name = reference->GetName();

	if (name.empty())
		return;

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetName", {reference->GetReferenceParam(), RawParameter(name)});

	Interface::EndDynamic();
}

void Game::SetRestrained(const FactoryObject<Actor>& reference, bool restrained)
{
	//bool restrained = actor->GetActorRestrained();

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetRestrained", {reference->GetReferenceParam(), RawParameter(restrained)});

	Interface::EndDynamic();
}

void Game::SetPos(const FactoryObject<Object>& reference)
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

void Game::SetAngle(const FactoryObject<Object>& reference)
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

void Game::MoveTo(const FactoryObject<Object>& reference, const FactoryObject<Object>& object, bool cell, unsigned int key)
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

void Game::SetActorValue(const FactoryObject<Actor>& reference, bool base, unsigned char index, unsigned int key)
{
	Interface::StartDynamic();

	if (base)
		Interface::ExecuteCommand("SetActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(reference->GetActorBaseValue(index))}, key);
	else
		Interface::ExecuteCommand("ForceActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(reference->GetActorValue(index))}, key);

	Interface::EndDynamic();
}

void Game::DamageActorValue(const FactoryObject<Actor>& reference, unsigned char index, double value, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("DamageActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

	Interface::EndDynamic();
}

void Game::RestoreActorValue(const FactoryObject<Actor>& reference, unsigned char index, double value, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("RestoreActorValue", {reference->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

	Interface::EndDynamic();
}

function<void()> Game::SetActorSneaking(const FactoryObject<Actor>& reference, unsigned int key)
{
	NetworkID id = reference->GetNetworkID();

	function<void()> sneaking = [=]
	{
		try
		{
			SetRestrained(GameFactory::GetObject<Actor>(id).get(), false);

			this_thread::sleep_for(chrono::milliseconds(20));

			auto reference = GameFactory::GetObject<Actor>(id);

			Interface::StartDynamic();

			Interface::ExecuteCommand("SetForceSneak", {reference->GetReferenceParam(), RawParameter(reference->GetActorSneaking())}, key);

			Interface::EndDynamic();

			GameFactory::LeaveReference(reference.get());

			this_thread::sleep_for(chrono::milliseconds(100));

			SetRestrained(GameFactory::GetObject<Actor>(id).get(), true);
		}
		catch (...) {}
	};

	return sneaking;
}

function<void()> Game::SetActorAlerted(const FactoryObject<Actor>& reference, unsigned int key)
{
	// really need to introduce restrained state in Actor class

	NetworkID id = reference->GetNetworkID();

	function<void()> alerted = [=]
	{
		try
		{
			SetRestrained(GameFactory::GetObject<Actor>(id).get(), false);

			this_thread::sleep_for(chrono::milliseconds(20));

			auto reference = GameFactory::GetObject<Actor>(id);

			Interface::StartDynamic();

			Interface::ExecuteCommand("SetAlert", {reference->GetReferenceParam(), RawParameter(reference->GetActorAlerted())}, key);

			Interface::EndDynamic();

			GameFactory::LeaveReference(reference.get());

			this_thread::sleep_for(chrono::milliseconds(100));

			SetRestrained(GameFactory::GetObject<Actor>(id).get(), true);
		}
		catch (...) {}
	};

	return alerted;
}

void Game::SetActorAnimation(const FactoryObject<Actor>& reference, unsigned char anim, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("PlayGroup", {reference->GetReferenceParam(), RawParameter(API::RetrieveAnim_Reverse(anim)), RawParameter(true)}, key);

	Interface::EndDynamic();
}

void Game::SetActorMovingAnimation(const FactoryObject<Actor>& reference, unsigned int key)
{
	SetActorAnimation(reference, reference->GetActorMovingAnimation(), key);
}

void Game::SetActorWeaponAnimation(const FactoryObject<Actor>& reference, unsigned int key)
{
	SetActorAnimation(reference, reference->GetActorWeaponAnimation(), key);
}

void Game::SetActorIdleAnimation(const FactoryObject<Actor>& reference, const string& anim, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("PlayIdle", {reference->GetReferenceParam(), RawParameter(anim)}, key);

	Interface::EndDynamic();
}

void Game::SetActorRace(const FactoryObject<Actor>& reference, signed int delta_age, unsigned int key)
{
	unsigned int baseID = reference->GetBase();
	unsigned int race = reference->GetActorRace();

	// set only once per base
	if (baseRaces[baseID] == race)
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	baseRaces[baseID] = race;

	Interface::StartDynamic();

	Interface::ExecuteCommand("MatchRace", {reference->GetReferenceParam(), RawParameter(reference->GetActorRace())}, delta_age ? 0 : key);

	if (delta_age)
		Interface::ExecuteCommand("AgeRace", {reference->GetReferenceParam(), RawParameter(delta_age)}, key);

	Interface::EndDynamic();
}

void Game::SetActorFemale(const FactoryObject<Actor>& reference, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("SexChange", {reference->GetReferenceParam(), RawParameter(reference->GetActorFemale())}, key);

	Interface::EndDynamic();
}

void Game::KillActor(const FactoryObject<Actor>& reference, unsigned short limbs, signed char cause, unsigned int key)
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

void Game::FireWeapon(const FactoryObject<Actor>& reference, unsigned int weapon, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("FireWeapon", {reference->GetReferenceParam(), RawParameter(weapon)}, key);

	Interface::EndDynamic();
}

void Game::AddItem(const FactoryObject<Container>& reference, const FactoryObject<Item>& item, unsigned int key)
{
	AddItem(reference, item->GetBase(), item->GetItemCount(), item->GetItemCondition(), item->GetItemSilent(), key);
}

void Game::AddItem(const FactoryObject<Container>& reference, unsigned int baseID, unsigned int count, double condition, bool silent, unsigned int key)
{
	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && reference->GetReference() == PLAYER_REFERENCE)
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	Interface::StartDynamic();

	Interface::ExecuteCommand("AddItemHealthPercent", {reference->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(condition / 100), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::RemoveItem(const FactoryObject<Container>& reference, const FactoryObject<Item>& item, unsigned int key)
{
	RemoveItem(reference, item->GetBase(), item->GetItemCount(), item->GetItemSilent(), key);
}

void Game::RemoveItem(const FactoryObject<Container>& reference, unsigned int baseID, unsigned int count, bool silent, unsigned int key)
{
	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && reference->GetReference() == PLAYER_REFERENCE)
	{
		if (key)
			Lockable::Retrieve(key);
		return;
	}

	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveItem", {reference->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::RemoveAllItems(const FactoryObject<Container>& reference, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveAllItems", {reference->GetReferenceParam()}, key);

	Interface::EndDynamic();
}

void Game::RemoveAllItemsEx(FactoryObject<Container>& reference)
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
		throw VaultException("Obtaining of all items of %llu for RemoveAllItemsEx failed (%s)", id, e.what());
	}
}

void Game::SetRefCount(const FactoryObject<Item>& reference, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("SetRefCount", {reference->GetReferenceParam(), RawParameter(reference->GetItemCount())}, key);

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
		throw VaultException("Obtaining of reference count of refID %08X (%s)", refID, e.what());
	}

	return count;
}

void Game::EquipItem(const FactoryObject<Actor>& reference, const FactoryObject<Item>& item, unsigned int key)
{
	EquipItem(reference, item->GetBase(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::EquipItem(const FactoryObject<Actor>& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
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

void Game::UnequipItem(const FactoryObject<Actor>& reference, const FactoryObject<Item>& item, unsigned int key)
{
	UnequipItem(reference, item->GetBase(), item->GetItemSilent(), item->GetItemStick(), key);
}

void Game::UnequipItem(const FactoryObject<Actor>& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
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
		throw VaultException("Scan of player cell with type %d failed (%s)", type, e.what());
	}

	return diff;
}

pair<ContainerDiffNet, GameDiff> Game::ScanContainer(FactoryObject<Container>& reference)
{
	auto store = make_shared<Shared<pair<ContainerDiffNet, GameDiff>>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("ScanContainer", {reference->GetReferenceParam()}, key);

	Interface::EndDynamic();

	NetworkID id = reference->GetNetworkID();
	GameFactory::LeaveReference(reference);

	pair<ContainerDiffNet, GameDiff> diff;

	try
	{
		diff = store->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Scan of container inventory %llu failed (%s)", id, e.what());
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
		throw VaultException("Respawning failed (%s)", e.what());
	}
}

void Game::net_SetPos(const FactoryObject<Object>& reference, double X, double Y, double Z)
{
	bool result = (static_cast<bool>(reference->SetNetworkPos(Axis_X, X)) | static_cast<bool>(reference->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(reference->SetNetworkPos(Axis_Z, Z)));

	if (result && reference->GetEnabled())
	{
		auto actor = vaultcast<Actor>(reference); // maybe we should consider items, too (they have physics)

		if (!actor || (!reference->IsNearPoint(reference->GetNetworkPos(Axis_X), reference->GetNetworkPos(Axis_Y), reference->GetNetworkPos(Axis_Z), 50.0)) || actor->IsActorJumping())
			SetPos(reference);
	}
}

void Game::net_SetAngle(const FactoryObject<Object>& reference, unsigned char axis, double value)
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

void Game::net_SetCell(const FactoryObject<Object>& reference, const FactoryObject<Player>& player, unsigned int cell)
{
	reference->SetNetworkCell(cell);

	if (reference != player)
	{
		if (reference->GetNetworkCell() != player->GetGameCell())
		{
			if (reference->SetEnabled(false))
				ToggleEnabled(reference);
		}
		else
		{
			if (reference->SetEnabled(true))
				ToggleEnabled(reference);
		}
	}
}

void Game::net_ContainerUpdate(FactoryObject<Container>& reference, const ContainerDiffNet& ndiff, const ContainerDiffNet& gdiff)
{
	Lockable* result;

	// cleaner solution here

	ContainerDiff diff = Container::ToContainerDiff(ndiff);
	NetworkID id = reference->GetNetworkID();
	auto container = reference.operator ->();
	GameFactory::LeaveReference(reference);

	while (!(result = container->getLock()));

	reference = GameFactory::GetObject<Container>(id).get();

	unsigned int key = result->Lock();

	GameDiff _gdiff = reference->ApplyDiff(diff);

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
		Delete(reference.get());
	}

	for (const auto& packet : gdiff.second)
	{
		NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet.get());
		auto reference = GameFactory::GetObject<Item>(id);
		reference->SetReference(0x00000000);
		NewItem(reference.get());
	}
}

void Game::net_SetActorValue(const FactoryObject<Actor>& reference, bool base, unsigned char index, double value)
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

void Game::net_SetActorState(const FactoryObject<Actor>& reference, unsigned int, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool firing)
{
	Lockable* result;
	bool enabled = reference->GetEnabled();

	result = reference->SetActorMovingXY(movingxy);

	if (result && enabled)
		SetAngle(reference);

	result = reference->SetActorAlerted(alerted);

	if (result && enabled)
		AsyncDispatch(SetActorAlerted(reference, result->Lock()));

	result = reference->SetActorSneaking(sneaking);

	if (result && enabled)
		AsyncDispatch(SetActorSneaking(reference, result->Lock()));

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
		//(game == FALLOUT3 ? (weapon == Fallout3::AnimGroup_BlockHit) : (weapon == FalloutNV::AnimGroup_BlockHit)) &&
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

void Game::net_SetActorRace(const FactoryObject<Actor>& reference, unsigned int race, signed int age, signed int delta_age)
{
	reference->SetActorRace(race);
	reference->SetActorAge(age); // delta from original race to new race
	SetActorRace(reference, delta_age); // using delta from current race to new race
}

void Game::net_SetActorFemale(const FactoryObject<Actor>& reference, bool female)
{
	reference->SetActorFemale(female);
	SetActorFemale(reference);
}

void Game::net_SetActorDead(FactoryObject<Actor>& reference, bool dead, unsigned short limbs, signed char cause)
{
	Lockable* result;

	result = reference->SetActorDead(dead);

	if (result)
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
			Game::LoadEnvironment();

			reference = GameFactory::GetObject<Actor>(id).get();
			reference->SetEnabled(true);
			GameFactory::LeaveReference(reference);

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, false, 0, 0),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::net_FireWeapon(const FactoryObject<Actor>& reference, unsigned int weapon, double rate)
{
	bool enabled = reference->GetEnabled();

	if (enabled)
	{
		FireWeapon(reference, weapon);
		NetworkID id = reference->GetNetworkID();

		if (rate)
		{
			// automatic weapons

			AsyncDispatch([=]
			{
				try
				{
					FactoryObject<Actor> reference;

					// rate: per second
					auto us = chrono::microseconds(static_cast<unsigned long long>(1000000ull / rate));

					this_thread::sleep_for(us);

					while ((reference = GameFactory::GetObject<Actor>(id).get()) && reference->IsActorFiring() && reference->IsEquipped(weapon))
					{
						FireWeapon(reference, weapon);
						GameFactory::LeaveReference(reference);
						this_thread::sleep_for(us);
					}
				}
				catch (...) {}
			});
		}
	}
}

void Game::net_SetActorIdle(const FactoryObject<Actor>& reference, unsigned int idle, const string& name)
{
	Lockable* result;
	bool enabled = reference->GetEnabled();

	result = reference->SetActorIdleAnimation(idle);

	if (result && enabled && idle)
		SetActorIdleAnimation(reference, name, result->Lock());
}

void Game::net_UIMessage(const string& message)
{
	UIMessage(message);
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

void Game::GetPos(const FactoryObject<Object>& reference, unsigned char axis, double value)
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

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(reference->GetNetworkID(), X, Y, Z),
				HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::GetAngle(const FactoryObject<Object>& reference, unsigned char axis, double value)
{
	bool result = static_cast<bool>(reference->SetAngle(axis, value));

	if (result)
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(reference->GetNetworkID(), axis, value),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
		});
}

void Game::GetParentCell(const FactoryObject<Object>& reference, const FactoryObject<Player>& player, unsigned int cell)
{
	if (reference != player)
	{
		if (player->GetGameCell() == reference->GetNetworkCell() && reference->GetGameCell() != reference->GetNetworkCell())
		{
			if (reference->SetEnabled(true))
				ToggleEnabled(reference);

			Lockable* result = reference->SetGameCell(player->GetGameCell());

			if (result)
				MoveTo(reference, player, true, result->Lock());

			SetAngle(reference);
		}
	}

	bool result = static_cast<bool>(reference->SetGameCell(cell));

	if (reference != player)
	{
		if (reference->GetNetworkCell() != player->GetGameCell())
		{
			if (reference->SetEnabled(false))
				ToggleEnabled(reference);
		}
		else
		{
			if (reference->SetEnabled(true))
				ToggleEnabled(reference);
		}
	}

	if (result && reference == player)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(reference->GetNetworkID(), cell),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
		});

		cellRefs.StartSession();

		for (unsigned int refID : (*cellRefs)[cell][FormType_Inventory])
		{
			auto reference = GameFactory::GetObject<Item>(refID); // probably potential deadlock. maybe copy set out

			if (!reference)
				continue; // we don't have information about static refs yet. remove

			auto& item = reference.get();

			if (item->GetNetworkCell() == cell && !item->GetEnabled())
			{
				item->SetEnabled(true);
				ToggleEnabled(item);

				if (item->SetGameCell(cell))
					MoveTo(item, player, true);
			}
		}

		cellRefs.EndSession();
/*
		AsyncDispatch([=]
		{
			debug->PrintFormat("new cell %08X", true, cell);

			this_thread::sleep_for(chrono::seconds(1));

			CellDiff diff = ScanCell(FormType_Inventory);

			for (auto r : diff.first)
				debug->PrintFormat("1 %08X", true, r);
			for (auto r : diff.second)
				debug->PrintFormat("2 %08X", true, r);
		});
*/
	}
}

void Game::GetDead(const FactoryObject<Actor>& reference, const FactoryObject<Player>&, bool dead)
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
							throw VaultException("Obtaining of limb data of %08X failed (%s)", refID, e.what());
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
							throw VaultException("Obtaining of cause of death of %08X failed (%s)", refID, e.what());
						}
					}

					Network::Queue(NetworkResponse{Network::CreateResponse(
						PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, true, limbs, cause),
						HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
					});
				}
				catch (...) {}
			});
		}
		else
		{
			Network::Queue(NetworkResponse{Network::CreateResponse(
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
		throw VaultException("Storage has expired");

	Shared<unsigned short>* store = dynamic_cast<Shared<unsigned short>*>(locked);

	if (store == nullptr)
		throw VaultException("Storage is corrupted");

	(**store) |= (static_cast<unsigned short>(gone) << limb);

	if (last_limb)
		store->set_promise();
}

void Game::GetActorValue(const FactoryObject<Actor>& reference, bool base, unsigned char index, double value)
{
	bool result;

	if (base)
		result = static_cast<bool>(reference->SetActorBaseValue(index, value));
	else
		result = static_cast<bool>(reference->SetActorValue(index, value));

	if (result)
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(reference->GetNetworkID(), base, index, value),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::GetActorState(const FactoryObject<Actor>& reference, unsigned int idle, unsigned char moving, unsigned char weapon, unsigned char flags, bool sneaking)
{
	static bool chat_state = false;
	static bool quit_state = true;
	static pair<unsigned char, unsigned char> buf_weapon{AnimGroup_Idle, AnimGroup_Idle};

	unsigned char chat_keys = flags >> 2;
	unsigned char movingxy = flags & 0x03;

	if (!chat_keys && !chat_state)
		quit_state = true;
	else if (chat_keys == 0x01 && !chat_state)
	{
		DisablePlayerControls(true, true, true, false);
		chat_state = true;
		quit_state = false;
	}
	else if ((chat_keys & (0x02 | 0x04)) && chat_state)
	{
		EnablePlayerControls();
		chat_state = false;
	}
	else if (chat_keys == 0x02 && quit_state)
	{
		Interface::SignalEnd();
		return;
	}

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
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(reference->GetNetworkID(), idle, moving, movingxy, reference->GetActorWeaponAnimation(), reference->GetActorAlerted(), sneaking, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::GetControl(const FactoryObject<Player>& reference, unsigned char control, unsigned char key)
{
	bool result = static_cast<bool>(reference->SetPlayerControl(control, key));

	if (result)
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTROL>(reference->GetNetworkID(), control, key),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::ScanContainer(const FactoryObject<Container>& reference, const vector<unsigned char>& data)
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

			gdiff.remove_if([](const pair<unsigned int, Diff>& diff) { return !diff.second.count; });

			if (!gdiff.empty())
			{
				// lambda can't capture by move :(

				auto _ndiff = make_shared<ContainerDiffNet>(move(ndiff));
				NetworkID id = reference->GetNetworkID();

				AsyncDispatch([_ndiff, gdiff, key, result, id]() mutable
				{
					try
					{
						{
							vector<NetworkID> reference = GameFactory::GetIDObjectTypes(ALL_CONTAINERS);
							unsigned int cell = vaultcast<Container>(GameFactory::GetObject(id))->GetGameCell();

							for (const NetworkID& _id : reference)
							{
								if (_id == id)
									continue;

								FactoryObject<Container> container = GameFactory::GetObject<Container>(_id).get();

								if (container->GetGameCell() != cell)
									continue;

								auto result = Game::ScanContainer(container);

								if (!result.first.first.empty() || !result.first.second.empty())
								{
									Network::Queue(NetworkResponse{Network::CreateResponse(
										PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(_id, result.first, ContainerDiffNet()),
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

							gdiff.remove_if([](const pair<unsigned int, Diff>& diff) { return !diff.second.count; });
						}

						ContainerDiffNet ndiff;

						if (!gdiff.empty())
						{
							CellDiff cdiff = ScanCell(FormType_Inventory);
							map<unsigned int, pair<GameDiff::iterator, list<pair<unsigned int, unsigned int>>>> found;

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

								auto it = find_if(gdiff.begin(), gdiff.end(), [=](const pair<unsigned int, Diff>& diff) { return diff.first == baseID; });

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
									FactoryObject<Container> reference = GameFactory::GetObject<Container>(id).get();

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
										FactoryObject<Item> reference = GameFactory::GetObject<Item>(id).get();

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

								auto it = find_if(gdiff.begin(), gdiff.end(), [=](const pair<unsigned int, Diff>& diff) { return diff.first == baseID; });

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

						Network::Queue(NetworkResponse{Network::CreateResponse(
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
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(reference->GetNetworkID(), ndiff, ContainerDiffNet()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
				});
		}

		if (key)
			result->Unlock(key);
	}
}

pair<ContainerDiffNet, GameDiff> Game::GetScanContainer(const FactoryObject<Container>& reference, const vector<unsigned char>& data)
{
	pair<ContainerDiffNet, GameDiff> result;

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

	FactoryObject<Container> temp = GameFactory::GetObject<Container>(GameFactory::CreateInstance(ID_CONTAINER, 0x00000000)).get();

	for (unsigned int i = 0; i < count; ++i)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(GameFactory::CreateInstance(ID_ITEM, items[i].baseID)).get();
		item->SetItemCount(items[i].count);
		item->SetItemEquipped(static_cast<bool>(items[i].equipped));
		item->SetItemCondition(items[i].condition);
		temp->AddItem(item->GetNetworkID());
	}

	ContainerDiff diff = reference->Compare(temp->GetNetworkID());

	if (!diff.first.empty() || !diff.second.empty())
	{
		result.first = Container::ToNetDiff(diff);
		result.second = reference->ApplyDiff(diff);
	}

	GameFactory::DestroyInstance(temp);

	return result;
}

void Game::GetRemoveAllItemsEx(const FactoryObject<Container>& reference, const vector<unsigned char>& data)
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

void Game::GetNextRef(unsigned int key, unsigned int refID, unsigned int type)
{
	static bool first = true;
	static unsigned int cell;
	static unsigned int _type;
	static set<unsigned int> data;

	if (first)
	{
		FactoryObject<Player> reference = GameFactory::GetObject<Player>(PLAYER_REFERENCE).get();
		cell = reference->GetGameCell();
		_type = type;
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
			throw VaultException("Storage has expired");

		Shared<CellDiff>* store = dynamic_cast<Shared<CellDiff>*>(locked);

		if (store == nullptr)
			throw VaultException("Storage is corrupted");

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

	Network::Queue(NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_CHAT>(message),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
	});
}
