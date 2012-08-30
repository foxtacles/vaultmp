#include "Game.h"
#include "PacketFactory.h"

unsigned char Game::game = 0x00;
RakNetGUID Game::server;

Game::CellRefs Game::cellRefs;
function<void()> Game::spawnFunc;

#ifdef VAULTMP_DEBUG
Debug* Game::debug;
#endif

using namespace Values;

#ifdef VAULTMP_DEBUG
void Game::SetDebugHandler(Debug* debug)
{
	Game::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Game class", true);
}
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
	unsigned short opcode = getFrom<double, unsigned short>(info.at(0));

	if (!error)
	{
#ifdef VAULTMP_DEBUG
		//debug->PrintFormat("Executing command %04hX on reference %08X, key %08X", true, opcode, info.size() > 1 ? getFrom<double, unsigned int>(info.at(1)) : 0, key);
#endif

		Lockable* data;
		weak_ptr<Lockable> shared;

		if (key)
		{
			switch (opcode)
			{
				case Func_ForceRespawn:
				case Func_RemoveAllItemsEx:
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
					data = Lockable::Retrieve(key);
			}
		}

		FactoryObject reference, self;

		switch (opcode)
		{
			case Func_PlaceAtMeHealthPercent:
				FutureSet<unsigned int>(shared, getFrom<double, unsigned int>(result));
				break;

			case Func_GetPos:
				reference = GameFactory::GetObject(getFrom<double, unsigned int>(info.at(1)));
				GetPos(reference, getFrom<double, unsigned char>(info.at(2)), result);
				break;

			case Func_SetPos:
				break;

			case Func_GetAngle:
				reference = GameFactory::GetObject(getFrom<double, unsigned int>(info.at(1)));
				GetAngle(reference, getFrom<double, unsigned char>(info.at(2)), result);
				break;

			case Func_SetAngle:
				break;

			case Func_GetActorValue:
				reference = GameFactory::GetObject(getFrom<double, unsigned int>(info.at(1)));
				Game::GetActorValue(reference, false, getFrom<double, unsigned char>(info.at(2)), result);
				break;

			case Func_ForceActorValue:
				break;

			case Func_DamageActorValue:
				break;

			case Func_RestoreActorValue:
				break;

			case Func_GetBaseActorValue:
				reference = GameFactory::GetObject(getFrom<double, unsigned int>(info.at(1)));
				GetActorValue(reference, true, getFrom<double, unsigned char>(info.at(2)), result);
				break;

			case Func_SetActorValue:
				break;

			case Func_GetActorState:
			{
				reference = GameFactory::GetObject(getFrom<double, unsigned int>(info.at(1)));
				GetActorState(reference,
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 7),
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 4),
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 5),
									*reinterpret_cast<unsigned char*>(((unsigned) &result) + 6),
									*reinterpret_cast<unsigned char*>(&result));
				break;
			}

			case Func_PlayGroup:
				break;

			case Func_GetDead:
			{
				vector<FactoryObject> objects = GameFactory::GetMultiple(vector<unsigned int>{getFrom<double, unsigned int>(info.at(1)), PLAYER_REFERENCE});
				GetDead(objects[0], objects[1], result);
				break;
			}

			case Func_IsLimbGone:
				IsLimbGone(key, getFrom<double, unsigned int>(info.at(2)), result);
				break;

			case Func_GetCauseofDeath:
				FutureSet<signed char>(shared, result);
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

				vector<unsigned char>& data = *getFrom<double, vector<unsigned char>*>(result);
				GetMessage(string(reinterpret_cast<char*>(&data[0]), data.size()));
				delete &data;
				break;
			}

			case Func_MarkForDelete:
				break;

			case Func_ScanContainer:
			{
				reference = GameFactory::GetObject(getFrom<double, unsigned int>(info.at(1)));
				vector<unsigned char>* data = getFrom<double, vector<unsigned char>*>(result);
				ScanContainer(reference, *data);
				delete data;
				break;
			}

			case Func_RemoveAllItemsEx:
			{
				reference = GameFactory::GetObject(getFrom<double, unsigned int>(info.at(1)));
				vector<unsigned char>* data = getFrom<double, vector<unsigned char>*>(result);
				GetRemoveAllItemsEx(reference, *data);
				FutureSet<bool>(shared, true);
				delete data;
				break;
			}

			case Fallout3::Func_GetBaseObject:
			case FalloutNV::Func_GetBaseObject:
				FutureSet<unsigned int>(shared, getFrom<double, unsigned int>(result));
				break;

			case Fallout3::Func_GetRefCount:
			case FalloutNV::Func_GetRefCount:
				FutureSet<unsigned int>(shared, result);
				break;

			case Fallout3::Func_SetRefCount:
			case FalloutNV::Func_SetRefCount:
				break;

			case Fallout3::Func_GetFirstRef:
			case FalloutNV::Func_GetFirstRef:
				GetNextRef(key, getFrom<double, unsigned int>(result), getFrom<double, unsigned int>(info.at(1)));
				break;

			case Fallout3::Func_GetNextRef:
			case FalloutNV::Func_GetNextRef:
				GetNextRef(key, getFrom<double, unsigned int>(result));
				break;

			case Func_UIMessage:
				break;

			case Fallout3::Func_GetParentCell:
			case FalloutNV::Func_GetParentCell:
			{
				vector<FactoryObject> objects = GameFactory::GetMultiple(vector<unsigned int>{getFrom<double, unsigned int>(info.at(1)), PLAYER_REFERENCE});
				GetParentCell(objects[0], objects[1], getFrom<double, unsigned int>(result));
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
				self = GameFactory::GetObject(PLAYER_REFERENCE);
				GetControl(self, getFrom<double, int>(info.at(1)), result);
				break;

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
				FutureSet<bool>(shared, true);
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
		if (debug)
			debug->PrintFormat("Command %04hX failed", true, opcode);
#endif

		switch (opcode)
		{
			case Func_PlaceAtMeHealthPercent:
				PlaceAtMe(getFrom<double, unsigned int>(info.at(1)), getFrom<double, unsigned int>(info.at(2)), getFrom<double, unsigned int>(info.at(3)), key);
				break;

			default:
				break;
		}
	}
}

NetworkResponse Game::Authenticate(string password)
{
	FactoryObject reference = GameFactory::GetObject(PLAYER_REFERENCE);
	Player* self = vaultcast<Player>(reference);

	return NetworkResponse{Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_AUTH>(self->GetName(), password),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
	};
}

void Game::Startup()
{
	FactoryObject reference = GameFactory::GetObject(PLAYER_REFERENCE);
	Player* self = vaultcast<Player>(reference);

	RawParameter self_ref = self->GetReferenceParam();
	NetworkID id = self->GetNetworkID();

	GameFactory::LeaveReference(reference);

	SetINISetting("bSaveOnInteriorExteriorSwitch:GamePlay", "0");
	SetINISetting("bSaveOnTravel:GamePlay", "0");
	SetINISetting("bSaveOnWait:GamePlay", "0");
	SetINISetting("bSaveOnRest:GamePlay", "0");

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
void Game::FutureSet(const weak_ptr<Lockable>& data, T t)
{
	shared_ptr<Lockable> shared = data.lock();
	Lockable* locked = shared.get();

	if (locked == nullptr)
		throw VaultException("Storage has expired");

	Shared<T>* store = dynamic_cast<Shared<T>*>(locked);

	if (store == nullptr)
		throw VaultException("Storage is corrupted");

	store->set(t);
	store->set_promise();
}
template void Game::FutureSet(const weak_ptr<Lockable>& data, unsigned int t);
template void Game::FutureSet(const weak_ptr<Lockable>& data, bool t);

void Game::AsyncDispatch(function<void()>&& func)
{
	thread t(func);
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
		store.get()->get_future(chrono::seconds(60));
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
		store.get()->get_future(chrono::seconds(60));
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
		store.get()->get_future(chrono::seconds(60));
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
		store.get()->get_future(chrono::seconds(60));
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

void Game::LoadEnvironment()
{
	vector<NetworkID> reference = GameFactory::GetIDObjectTypes(ALL_OBJECTS);

	for (NetworkID& id : reference)
	{
		FactoryObject reference = GameFactory::GetObject(id);

		if (!reference->IsPersistent())
		{
			// TODO critical section
			Object* object = vaultcast<Object>(reference);
			cellRefs[object->GetNetworkCell()][FormType_Inventory].erase(object->GetReference());
			object->SetReference(0x00000000);
		}

		unsigned char type = reference.GetType();

		switch (type)
		{
			case ID_OBJECT:
				NewObject(reference);
				break;

			case ID_ITEM:
				if (!vaultcast<Item>(reference)->GetItemContainer())
					NewItem(reference);
				break;

			case ID_CONTAINER:
				NewContainer(reference);
				break;

			case ID_ACTOR:
				NewActor(reference);
				break;

			case ID_PLAYER:
				NewPlayer(reference);
				break;

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

void Game::NewObject(FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	if (!object->GetReference())
	{
		auto store = make_shared<Shared<unsigned int>>();
		unsigned int key = Lockable::Share(store);

		Item* item = vaultcast<Item>(reference);
		double condition = item ? (item->GetItemCondition() / 100.0) : 1.00;

		PlaceAtMe(PLAYER_REFERENCE, object->GetBase(), condition, 1, key);

		NetworkID id = object->GetNetworkID();

		GameFactory::LeaveReference(reference);

		unsigned int refID;

		try
		{
			refID = store.get()->get_future(chrono::seconds(15));
		}
		catch (exception& e)
		{
			throw VaultException("Object creation with baseID %08X and NetworkID %llu failed (%s)", object->GetBase(), object->GetNetworkID(), e.what());
		}

		reference = GameFactory::GetObject(id);
		object = vaultcast<Object>(reference);

		object->SetReference(refID);
	}
	//else if (!(not in player cell) || !object->GetChanged())
		//return;

	unsigned int refID = object->GetReference();

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

				Object* object = vaultcast<Object>(objects[0]);
				Player* player = vaultcast<Player>(objects[1]);

				unsigned int cell = player->GetGameCell();

				if (object->GetNetworkCell() == cell)
				{
					MoveTo(objects[0], objects[1], true);
					object->SetEnabled(true);
				}
				else
				{
					object->SetEnabled(false);
					ToggleEnabled(objects[0]);
				}

				object->SetGameCell(cell);
			}
			catch (...) {}
		});
	}

	// maybe more
}

void Game::NewItem(FactoryObject& reference)
{
	Item* item = vaultcast<Item>(reference);
	NetworkID id = item->GetItemContainer();

	if (id)
		throw VaultException("Cannot create item %llu which is bound to a Container (%llu)", item->GetNetworkID(), id);

	NewObject(reference);
	SetRefCount(reference);

	// TODO critical section
	cellRefs[item->GetNetworkCell()][FormType_Inventory].insert(item->GetReference());
}

void Game::NewContainer(FactoryObject& reference)
{
	NewObject(reference);
	RemoveAllItemsEx(reference);

	Container* container = vaultcast<Container>(reference);
	vector<FactoryObject> items = GameFactory::GetMultiple(vector<NetworkID>(container->GetItemList().begin(), container->GetItemList().end()));

	for (const FactoryObject& _item : items)
	{
		AddItem(reference, _item);
		Item* item = vaultcast<Item>(_item);

		//debug->PrintFormat("ID: %llu, %s, %08X, %d, %d, %d, %d", true, item->GetNetworkID(), item->GetName().c_str(), item->GetBase(), (int)item->GetItemEquipped(), (int)item->GetItemSilent(), (int)item->GetItemStick(), item->GetItemCount());

		if (item->GetItemEquipped())
			EquipItem(reference, _item);
	}
}

void Game::NewActor(FactoryObject& reference)
{
	NewContainer(reference);

	vector<unsigned char> values = API::RetrieveAllValues();

	for (unsigned char value : values)
	{
		SetActorValue(reference, true, value);
		SetActorValue(reference, false, value);
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor->GetReference() != PLAYER_REFERENCE)
	{
		SetRestrained(reference, true);

		if (actor->GetActorAlerted())
			SetActorAlerted(reference)();

		if (actor->GetActorSneaking())
			SetActorSneaking(reference)();

		if (actor->GetActorMovingAnimation() != AnimGroup_Idle)
			SetActorMovingAnimation(reference);

		if (actor->GetActorWeaponAnimation() != AnimGroup_Idle)
			SetActorWeaponAnimation(reference);
	}
}

void Game::NewPlayer(FactoryObject& reference)
{
	NewActor(reference);

	// ...
}

void Game::RemoveObject(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	if (object->SetEnabled(false))
		ToggleEnabled(reference);

	Interface::StartDynamic();

	Interface::ExecuteCommand("MarkForDelete", {object->GetReferenceParam()});

	Interface::EndDynamic();

	// TODO critical section
	cellRefs[object->GetNetworkCell()][FormType_Inventory].erase(object->GetReference());
}

void Game::PlaceAtMe(const FactoryObject& reference, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	PlaceAtMe(container->GetReference(), baseID, condition, count, key);
}

void Game::PlaceAtMe(unsigned int refID, unsigned int baseID, double condition, unsigned int count, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("PlaceAtMeHealthPercent", {RawParameter(refID), RawParameter(baseID), RawParameter(condition), RawParameter(count)}, key);

	Interface::EndDynamic();
}

void Game::ToggleEnabled(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	Interface::StartDynamic();

	if (object->GetEnabled())
		Interface::ExecuteCommand("Enable", {object->GetReferenceParam(), RawParameter(true)});
	else
		Interface::ExecuteCommand("Disable", {object->GetReferenceParam(), RawParameter(false)});

	Interface::EndDynamic();
}

void Game::Delete(FactoryObject& reference)
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
		baseID = store.get()->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Obtaining of baseID of refID %08X (%s)", refID, e.what());
	}

	return baseID;
}

void Game::SetName(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);
	string name = object->GetName();

	if (name.empty())
		return;

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetName", {object->GetReferenceParam(), RawParameter(name)});

	Interface::EndDynamic();
}

void Game::SetRestrained(const FactoryObject& reference, bool restrained)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	//bool restrained = actor->GetActorRestrained();

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetRestrained", {actor->GetReferenceParam(), RawParameter(restrained)});

	Interface::EndDynamic();
}

void Game::SetPos(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	if (!object->HasValidCoordinates())
		return;

	Lockable* key = nullptr;

	Interface::StartDynamic();

	key = object->SetGamePos(Axis_X, object->GetNetworkPos(Axis_X));

	Interface::ExecuteCommand("SetPos", {object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(object->GetNetworkPos(Axis_X))}, key ? key->Lock() : 0);

	key = object->SetGamePos(Axis_Y, object->GetNetworkPos(Axis_Y));

	Interface::ExecuteCommand("SetPos", {object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Y)), RawParameter(object->GetNetworkPos(Axis_Y))}, key ? key->Lock() : 0);

	key = object->SetGamePos(Axis_Z, object->GetNetworkPos(Axis_Z));

	Interface::ExecuteCommand("SetPos", {object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(object->GetNetworkPos(Axis_Z))}, key ? key->Lock() : 0);

	Interface::EndDynamic();
}

void Game::SetAngle(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetAngle", {object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(object->GetAngle(Axis_X))});

	double value = object->GetAngle(Axis_Z);
	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
	{
		if (actor->GetActorMovingXY() == 0x01)
			AdjustZAngle(value, -45.0);

		else if (actor->GetActorMovingXY() == 0x02)
			AdjustZAngle(value, 45.0);
	}

	Interface::ExecuteCommand("SetAngle", {object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(value)});

	Interface::EndDynamic();
}

void Game::MoveTo(const FactoryObject& reference, const FactoryObject& object, bool cell, unsigned int key)
{
	Object* _object = vaultcast<Object>(reference);
	Object* _object2 = vaultcast<Object>(object);

	Interface::StartDynamic();

	ParamContainer param_MoveTo{_object->GetReferenceParam(), _object2->GetReferenceParam()};

	if (cell)
	{
		param_MoveTo.emplace_back(_object->GetNetworkPos(Axis_X) - _object2->GetNetworkPos(Axis_X));
		param_MoveTo.emplace_back(_object->GetNetworkPos(Axis_Y) - _object2->GetNetworkPos(Axis_Y));
		param_MoveTo.emplace_back(_object->GetNetworkPos(Axis_Z) - _object2->GetNetworkPos(Axis_Z));
	}

	Interface::ExecuteCommand("MoveTo", move(param_MoveTo), key);

	Interface::EndDynamic();
}

void Game::SetActorValue(const FactoryObject& reference, bool base, unsigned char index, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	if (base)
		Interface::ExecuteCommand("SetActorValue", {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(actor->GetActorBaseValue(index))}, key);
	else
		Interface::ExecuteCommand("ForceActorValue", {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(actor->GetActorValue(index))}, key);

	Interface::EndDynamic();
}

void Game::DamageActorValue(const FactoryObject& reference, unsigned char index, double value, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("DamageActorValue", {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

	Interface::EndDynamic();
}

void Game::RestoreActorValue(const FactoryObject& reference, unsigned char index, double value, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("RestoreActorValue", {actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(value)}, key);

	Interface::EndDynamic();
}

function<void()> Game::SetActorSneaking(const FactoryObject& reference, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	NetworkID id = actor->GetNetworkID();

	function<void()> sneaking = [=]
	{
		try
		{
			SetRestrained(GameFactory::GetObject(id), false);

			this_thread::sleep_for(chrono::milliseconds(20));

			FactoryObject reference = GameFactory::GetObject(id);
			Actor* actor = vaultcast<Actor>(reference);

			Interface::StartDynamic();

			Interface::ExecuteCommand("SetForceSneak", {actor->GetReferenceParam(), RawParameter(actor->GetActorSneaking())}, key);

			Interface::EndDynamic();

			GameFactory::LeaveReference(reference);

			this_thread::sleep_for(chrono::milliseconds(100));

			SetRestrained(GameFactory::GetObject(id), true);
		}
		catch (...) {}
	};

	return sneaking;
}

function<void()> Game::SetActorAlerted(const FactoryObject& reference, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	// really need to introduce restrained state in Actor class

	NetworkID id = actor->GetNetworkID();

	function<void()> alerted = [=]
	{
		try
		{
			SetRestrained(GameFactory::GetObject(id), false);

			this_thread::sleep_for(chrono::milliseconds(20));

			FactoryObject reference = GameFactory::GetObject(id);
			Actor* actor = vaultcast<Actor>(reference);

			Interface::StartDynamic();

			Interface::ExecuteCommand("SetAlert", {actor->GetReferenceParam(), RawParameter(actor->GetActorAlerted())}, key);

			Interface::EndDynamic();

			GameFactory::LeaveReference(reference);

			this_thread::sleep_for(chrono::milliseconds(100));

			SetRestrained(GameFactory::GetObject(id), true);
		}
		catch (...) {}
	};

	return alerted;
}

void Game::SetActorAnimation(const FactoryObject& reference, unsigned char anim, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("PlayGroup", {actor->GetReferenceParam(), RawParameter(API::RetrieveAnim_Reverse(anim)), RawParameter(true)}, key);

	Interface::EndDynamic();
}

void Game::SetActorMovingAnimation(const FactoryObject& reference, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	SetActorAnimation(reference, actor->GetActorMovingAnimation(), key);
}

void Game::SetActorWeaponAnimation(const FactoryObject& reference, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	SetActorAnimation(reference, actor->GetActorWeaponAnimation(), key);
}

void Game::KillActor(const FactoryObject& reference, unsigned short limbs, signed char cause, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	// maybe add valid killer later
	if (limbs)
	{
		for (unsigned int i = 1; i <= limbs; i <<= 1)
			if (limbs & i)
				Interface::ExecuteCommand("Kill", {actor->GetReferenceParam(), actor->GetReferenceParam(), RawParameter(static_cast<unsigned int>(i / 2)), RawParameter(cause)}, ((i << 1) > limbs) ? key : 0x00);
	}
	else
		Interface::ExecuteCommand("Kill", {actor->GetReferenceParam(), actor->GetReferenceParam(), RawParameter(Limb_None), RawParameter(cause)}, key);

	Interface::EndDynamic();
}

void Game::FireWeapon(const FactoryObject& reference, unsigned int weapon, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("FireWeapon", {actor->GetReferenceParam(), RawParameter(weapon)}, key);

	Interface::EndDynamic();
}

void Game::AddItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", item->GetReference());

	AddItem(reference, _item->GetBase(), _item->GetItemCount(), _item->GetItemCondition(), _item->GetItemSilent(), key);
}

void Game::AddItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, double condition, bool silent, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("AddItemHealthPercent", {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(condition / 100), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::RemoveItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", item->GetReference());

	RemoveItem(reference, _item->GetBase(), _item->GetItemCount(), _item->GetItemSilent(), key);
}

void Game::RemoveItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, bool silent, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveItem", {container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::RemoveAllItems(const FactoryObject& reference, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveAllItems", {container->GetReferenceParam()}, key);

	Interface::EndDynamic();
}

void Game::RemoveAllItemsEx(FactoryObject& reference)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveAllItemsEx", {container->GetReferenceParam()}, key);

	Interface::EndDynamic();

	NetworkID id = container->GetNetworkID();
	GameFactory::LeaveReference(reference);

	try
	{
		store.get()->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Obtaining of all items of %llu for RemoveAllItemsEx failed (%s)", id, e.what());
	}

	reference = GameFactory::GetObject(id);
}

void Game::SetRefCount(const FactoryObject& reference, unsigned int key)
{
	Item* item = vaultcast<Item>(reference);

	if (!item)
		throw VaultException("Object with reference %08X is not an Item", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetRefCount", {item->GetReferenceParam(), RawParameter(item->GetItemCount())}, key);

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
		count = store.get()->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Obtaining of reference count of refID %08X (%s)", refID, e.what());
	}

	return count;
}

void Game::EquipItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", item->GetReference());

	EquipItem(reference, _item->GetBase(), _item->GetItemSilent(), _item->GetItemStick(), key);
}

void Game::EquipItem(const FactoryObject& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("EquipItem", {actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::UnequipItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", item->GetReference());

	UnequipItem(reference, _item->GetBase(), _item->GetItemSilent(), _item->GetItemStick(), key);
}

void Game::UnequipItem(const FactoryObject& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("UnequipItem", {actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

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
		diff = store.get()->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Scan of player cell with type %d failed (%s)", type, e.what());
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

void Game::ForceRespawn()
{
	auto store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("ForceRespawn", {}, key);

	Interface::EndDynamic();

	try
	{
		store.get()->get_future(chrono::seconds(5));
	}
	catch (exception& e)
	{
		throw VaultException("Respawning failed (%s)", e.what());
	}
}

void Game::net_SetPos(const FactoryObject& reference, double X, double Y, double Z)
{
	Object* object = vaultcast<Object>(reference);
	bool result = (static_cast<bool>(object->SetNetworkPos(Axis_X, X)) | static_cast<bool>(object->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(object->SetNetworkPos(Axis_Z, Z)));

	if (result && object->GetEnabled())
	{
		Actor* actor = vaultcast<Actor>(reference);   // maybe we should consider items, too (they have physics)

		if (actor == nullptr || (!actor->IsNearPoint(object->GetNetworkPos(Axis_X), object->GetNetworkPos(Axis_Y), object->GetNetworkPos(Axis_Z), 50.0)) || actor->IsActorJumping())
			SetPos(reference);
	}
}

void Game::net_SetAngle(const FactoryObject& reference, unsigned char axis, double value)
{
	Object* object = vaultcast<Object>(reference);
	bool result = static_cast<bool>(object->SetAngle(axis, value));

	if (result && object->GetEnabled())
	{
		SetAngle(reference);

		if (axis == Axis_X)
		{
			Actor* actor = vaultcast<Actor>(reference);

			if (actor && actor->GetActorWeaponAnimation() == AnimGroup_AimIS)
			{
				SetActorAnimation(reference, AnimGroup_AimISDown);
				SetActorAnimation(reference, AnimGroup_AimISUp);
			}
		}
	}
}

void Game::net_SetCell(const FactoryObject& reference, const FactoryObject& player, unsigned int cell)
{
	Object* object = vaultcast<Object>(reference);
	Player* self = vaultcast<Player>(player);

	if (!self)
		throw VaultException("Object with reference %08X is not a Player", player->GetReference());

	object->SetNetworkCell(cell);

	if (object != self)
	{
		if (object->GetNetworkCell() != self->GetGameCell())
		{
			if (object->SetEnabled(false))
				ToggleEnabled(reference);
		}
		else
		{
			if (object->SetEnabled(true))
				ToggleEnabled(reference);
		}
	}
}

void Game::net_ContainerUpdate(FactoryObject& reference, const pair<list<NetworkID>, vector<pPacket>>& ndiff, const pair<list<NetworkID>, vector<pPacket>>& gdiff)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	Lockable* result;

	// cleaner solution here

	ContainerDiff diff = Container::ToContainerDiff(ndiff);
	NetworkID id = container->GetNetworkID();
	GameFactory::LeaveReference(reference);

	while (!(result = container->getLock()));

	reference = GameFactory::GetObject(id);

	unsigned int key = result->Lock();

	GameDiff _gdiff = container->ApplyDiff(diff);

	for (const auto& diff : _gdiff)
	{
		if (diff.second.equipped)
		{
			if (diff.second.equipped > 0)
				EquipItem(reference, diff.first, diff.second.silent, diff.second.stick, result->Lock());
			else if (diff.second.equipped < 0)
				UnequipItem(reference, diff.first, diff.second.silent, diff.second.stick, result->Lock());
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
		FactoryObject reference = GameFactory::GetObject(id);
		Delete(reference);
	}

	for (const auto& packet : gdiff.second)
	{
		NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet.get());
		FactoryObject reference = GameFactory::GetObject(id);
		vaultcast<Item>(reference)->SetReference(0x00000000);
		NewItem(reference);
	}
}

void Game::net_SetActorValue(const FactoryObject& reference, bool base, unsigned char index, double value)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Lockable* result;

	double prev_value = actor->GetActorValue(index);

	if (base)
		result = actor->SetActorBaseValue(index, value);
	else
		result = actor->SetActorValue(index, value);

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

void Game::net_SetActorState(const FactoryObject& reference, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool firing)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Lockable* result;
	bool enabled = actor->GetEnabled();

	result = actor->SetActorMovingXY(movingxy);

	if (result && enabled)
		SetAngle(reference);

	result = actor->SetActorAlerted(alerted);

	if (result && enabled)
		AsyncDispatch(SetActorAlerted(reference, result->Lock()));

	result = actor->SetActorSneaking(sneaking);

	if (result && enabled)
		AsyncDispatch(SetActorSneaking(reference, result->Lock()));

	result = actor->SetActorMovingAnimation(moving);

	if (result && enabled)
	{
		SetActorMovingAnimation(reference, result->Lock());

		if (moving == AnimGroup_Idle)
			SetPos(reference);
	}

	unsigned char prev_weapon = actor->GetActorWeaponAnimation();
	result = actor->SetActorWeaponAnimation(weapon);

	if (result && enabled && actor->GetActorAlerted() && weapon != AnimGroup_Idle && weapon != AnimGroup_Equip && weapon != AnimGroup_Unequip && weapon != AnimGroup_Holster &&
		!firing && (weapon != AnimGroup_Aim || prev_weapon == AnimGroup_AimIS))
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

void Game::net_SetActorDead(FactoryObject& reference, bool dead, unsigned short limbs, signed char cause)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Lockable* result;

	result = actor->SetActorDead(dead);

	if (result)
	{
		if (dead)
			KillActor(reference, limbs, cause, result->Lock());
		else if (actor->GetReference() != PLAYER_REFERENCE)
		{
			RemoveObject(reference);
			actor->SetReference(0x00000000);
			NewActor(reference);
		}
		else
		{
			NetworkID id = actor->GetNetworkID();
			actor->SetEnabled(false);
			GameFactory::LeaveReference(reference);

			Game::ForceRespawn();

			this_thread::sleep_for(chrono::seconds(1));

			Game::spawnFunc();
			Game::LoadEnvironment();

			reference = GameFactory::GetObject(id);
			vaultcast<Actor>(reference)->SetEnabled(true);
			GameFactory::LeaveReference(reference);

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, false, 0, 0),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::net_FireWeapon(const FactoryObject& reference, unsigned int weapon, double rate)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	FireWeapon(reference, weapon);
	NetworkID id = actor->GetNetworkID();

	if (rate)
	{
		// automatic weapons

		AsyncDispatch([=]
		{
			try
			{
				FactoryObject reference;
				Actor* actor;

				// rate: per second
				auto us = chrono::microseconds(static_cast<unsigned long long>(1000000ull / rate));

				this_thread::sleep_for(us);

				while ((actor = vaultcast<Actor>(reference = GameFactory::GetObject(id))) && actor->IsActorFiring() && actor->IsEquipped(weapon))
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

void Game::net_UIMessage(const string& message)
{
	UIMessage(message);
}

void Game::net_ChatMessage(const string& message)
{
	ChatMessage(message);
}

void Game::GetPos(const FactoryObject& reference, unsigned char axis, double value)
{
	static bool update = false;

	Object* object = vaultcast<Object>(reference);
	bool result = static_cast<bool>(object->SetGamePos(axis, value));

	if (object->GetReference() == PLAYER_REFERENCE)
	{
		update |= result;

		if (axis == Axis_Z && update)
		{
			update = false;

			double X = object->GetGamePos(Axis_X);
			double Y = object->GetGamePos(Axis_Y);
			double Z = object->GetGamePos(Axis_Z);

			object->SetNetworkPos(Axis_X, X);
			object->SetNetworkPos(Axis_Y, Y);
			object->SetNetworkPos(Axis_Z, Z);

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(object->GetNetworkID(), X, Y, Z),
				HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::GetAngle(const FactoryObject& reference, unsigned char axis, double value)
{
	Object* object = vaultcast<Object>(reference);
	bool result = static_cast<bool>(object->SetAngle(axis, value));

	if (result)
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(object->GetNetworkID(), axis, value),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
		});
}

void Game::GetParentCell(const FactoryObject& reference, const FactoryObject& player, unsigned int cell)
{
	Object* object = vaultcast<Object>(reference);
	Player* self = vaultcast<Player>(player);

	if (!self)
		throw VaultException("Object with reference %08X is not a Player", player->GetReference());

	if (object != self)
	{
		if (self->GetGameCell() == object->GetNetworkCell() && object->GetGameCell() != object->GetNetworkCell())
		{
			if (object->SetEnabled(true))
				ToggleEnabled(reference);

			Lockable* result = object->SetGameCell(self->GetGameCell());

			if (result)
				MoveTo(reference, player, true, result->Lock());

			SetAngle(reference);
		}
	}

	bool result = static_cast<bool>(object->SetGameCell(cell));

	if (object != self)
	{
		if (object->GetNetworkCell() != self->GetGameCell())
		{
			if (object->SetEnabled(false))
				ToggleEnabled(reference);
		}
		else
		{
			if (object->SetEnabled(true))
				ToggleEnabled(reference);
		}
	}

	if (result && object == self)
	{
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(object->GetNetworkID(), cell),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
		});

		// TODO CS

		for (unsigned int refID : cellRefs[cell][FormType_Inventory])
		{
			FactoryObject _item;

			try
			{
				_item = GameFactory::GetObject(refID);
			}
			catch (...)
			{
				// we don't have information about static refs yet. remove
				continue;
			}

			Item* item = vaultcast<Item>(_item);

			if (!item->GetEnabled() && item->GetNetworkCell() == cell)
			{
				item->SetEnabled(true);
				ToggleEnabled(_item);

				if (item->SetGameCell(cell))
					MoveTo(_item, player, true);
			}
		}
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

void Game::GetDead(const FactoryObject& reference, const FactoryObject& player, bool dead)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	Player* self = vaultcast<Player>(player);

	if (!self)
		throw VaultException("Object with reference %08X is not a Player", player->GetReference());

	/*if (actor != self && !self->GetActorAlerted())
	{
	    // "bug death"

	    return;
	}*/

	bool result;

	result = static_cast<bool>(actor->SetActorDead(dead));

	if (result)
	{
		if (dead)
		{
			NetworkID id = actor->GetNetworkID();

			AsyncDispatch([=]
			{
				try
				{
					unsigned int refID;

					{
						FactoryObject reference = GameFactory::GetObject(id);
						Actor* actor = vaultcast<Actor>(reference);
						refID = actor->GetReference();
					}

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
							limbs = store.get()->get_future(chrono::seconds(5));
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
							cause = store.get()->get_future(chrono::seconds(5));
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
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(actor->GetNetworkID(), false, 0, 0),
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

	store->set(store->get() | (static_cast<unsigned short>(gone) << limb));

	if (last_limb)
		store->set_promise();
}

void Game::GetActorValue(const FactoryObject& reference, bool base, unsigned char index, double value)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	bool result;

	if (base)
		result = static_cast<bool>(actor->SetActorBaseValue(index, value));
	else
		result = static_cast<bool>(actor->SetActorValue(index, value));

	if (result)
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(actor->GetNetworkID(), base, index, value),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::GetActorState(const FactoryObject& reference, unsigned char chat_keys, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool sneaking)
{
	Actor* actor  = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	static bool chat_state = false;
	static bool quit_state = true;

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

	result = (static_cast<bool>(actor->SetActorMovingAnimation(moving)) | static_cast<bool>(actor->SetActorMovingXY(movingxy)) | static_cast<bool>(actor->SetActorSneaking(sneaking)));

	if (static_cast<bool>(actor->SetActorWeaponAnimation(weapon)))
	{
		result = true;

		if (weapon == AnimGroup_Equip)
			actor->SetActorAlerted(true);
		else if (weapon == AnimGroup_Unequip)
			actor->SetActorAlerted(false);
	}

	if (result)
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), moving, movingxy, weapon, actor->GetActorAlerted(), sneaking, false),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::GetControl(const FactoryObject& reference, unsigned char control, unsigned char key)
{
	Player* player = vaultcast<Player>(reference);

	if (!player)
		throw VaultException("Object with reference %08X is not a Player", reference->GetReference());

	bool result;

	result = static_cast<bool>(player->SetPlayerControl(control, key));

	if (result)
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTROL>(player->GetNetworkID(), control, key),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::ScanContainer(const FactoryObject& reference, vector<unsigned char>& data)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	Lockable* result;

	if ((result = container->getLock()))
	{
		unsigned int key = result->Lock();

#pragma pack(push, 1)
		struct ItemInfo
		{
			unsigned int baseID;
			unsigned int count;
			unsigned int equipped;
			double condition;
		};
#pragma pack(pop)

		ItemInfo* items = reinterpret_cast<ItemInfo*>(&data[0]);
		unsigned int count = data.size() / sizeof(ItemInfo);

		FactoryObject _temp = GameFactory::GetObject(GameFactory::CreateInstance(ID_CONTAINER, 0x00000000));
		Container* temp = vaultcast<Container>(_temp);

		for (unsigned int i = 0; i < count; ++i)
		{
			FactoryObject _item = GameFactory::GetObject(GameFactory::CreateInstance(ID_ITEM, items[i].baseID));
			Item* item = vaultcast<Item>(_item);
			item->SetItemCount(items[i].count);
			item->SetItemEquipped(static_cast<bool>(items[i].equipped));
			item->SetItemCondition(items[i].condition);
			temp->AddItem(item->GetNetworkID());
		}

		ContainerDiff diff = container->Compare(temp->GetNetworkID());

		if (!diff.first.empty() || !diff.second.empty())
		{
			ContainerDiffNet ndiff = Container::ToNetDiff(diff);
			GameDiff gdiff = container->ApplyDiff(diff);

			gdiff.remove_if([](const pair<unsigned int, Diff>& diff) { return !diff.second.count; });

			if (!gdiff.empty())
			{
				// lambda can't capture by move :(

				auto _ndiff = make_shared<ContainerDiffNet>(move(ndiff));
				NetworkID id = container->GetNetworkID();

				AsyncDispatch([_ndiff, gdiff, id]() mutable
				{
					try
					{
						ContainerDiffNet ndiff;
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
									else if (debug)
										debug->PrintFormat("Item match (drop): could not match %08X (baseID: %08X), count %d", true, refID, baseID, count);
#endif
								}
							}
#ifdef VAULTMP_DEBUG
							else if (debug)
								debug->PrintFormat("Item match (drop): could not match %08X (baseID: %08X) at all", true, refID, baseID);
#endif
						}

						if (!found.empty())
						{
							double X, Y, Z;
							unsigned int cell;

							{
								FactoryObject reference = GameFactory::GetObject(id);
								Container* container = vaultcast<Container>(reference);

								static const double spawn_offset = 100.0;

								// maybe better synchronically obtain XYZ for correctness
								auto offset = container->GetOffset(spawn_offset);

								X = offset.first;
								Y = offset.second;
								Z = container->GetGamePos(Axis_Z) + 70.0;

								cell = container->GetGameCell();
							}

							for (auto& _found : found)
							{
								auto result = best_match(_found);

#ifdef VAULTMP_DEBUG
								if (debug)
								{
									unsigned int count = abs(_found.second.first->second.count);

									if (!result.second.empty())
										debug->PrintFormat("Player dropped %08X (count %d, stacks %d)", true, _found.first, count, result.second.size());
									else
										debug->PrintFormat("Could not find a matching set for item drop %08X (count %d)", true, _found.first, count);
								}
#endif

								for (const auto& _result : result.second)
								{
									NetworkID id = GameFactory::CreateInstance(ID_ITEM, _result.first, _found.first);
									FactoryObject reference = GameFactory::GetObject(id);

									Item* item = vaultcast<Item>(reference);

									item->SetGamePos(Axis_X, X);
									item->SetGamePos(Axis_Y, Y);
									item->SetGamePos(Axis_Z, Z);
									item->SetNetworkPos(Axis_X, X);
									item->SetNetworkPos(Axis_Y, Y);
									item->SetNetworkPos(Axis_Z, Z);
									item->SetNetworkCell(cell);
									item->SetGameCell(cell);

									item->SetItemCount(_result.second);
									item->SetItemCondition(_found.second.first->second.condition);

									ndiff.second.emplace_back(item->toPacket());
								}

								gdiff.erase(_found.second.first);
							}
						}

						found.clear();

						for (unsigned int refID : cdiff.second)
						{
							FactoryObject reference;

							try
							{
								reference = GameFactory::GetObject(refID);
							}
							catch (...)
							{
#ifdef VAULTMP_DEBUG
								if (debug)
									debug->PrintFormat("Item match (pickup): could not find %08X", true, refID);
#endif
								continue;
							}

							Item* item = vaultcast<Item>(reference);

							unsigned int baseID = item->GetBase();

							auto it = find_if(gdiff.begin(), gdiff.end(), [=](const pair<unsigned int, Diff>& diff) { return diff.first == baseID; });

							if (it != gdiff.end())
							{
								if (it->second.count > 0)
								{
									unsigned int count = item->GetItemCount();

									if (it->second.count - static_cast<signed int>(count) >= 0)
									{
										auto& data = found[baseID];
										data.first = it;
										data.second.emplace_back(refID, count);
									}
#ifdef VAULTMP_DEBUG
									else if (debug)
										debug->PrintFormat("Item match (pickup): could not match %08X (baseID: %08X), count %d", true, refID, baseID, count);
#endif
								}
							}
#ifdef VAULTMP_DEBUG
							else if (debug)
								debug->PrintFormat("Item match (pickup): could not match %08X (baseID: %08X) at all", true, refID, baseID);
#endif
						}

						if (!found.empty())
						{
							for (auto& _found : found)
							{
								auto result = best_match(_found);

#ifdef VAULTMP_DEBUG
								if (debug)
								{
									unsigned int count = abs(_found.second.first->second.count);

									if (!result.second.empty())
										debug->PrintFormat("Player picked up %08X (count %d, stacks %d)", true, _found.first, count, result.second.size());
									else
										debug->PrintFormat("Could not find a matching set for item pickup %08X (count %d)", true, _found.first, count);
								}
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
						if (debug)
						{
							for (const auto& _gdiff : gdiff)
								debug->PrintFormat("Could not match drop / pickup %08X (count %d)", true, _gdiff.first, _gdiff.second.count);
						}
#endif

						Network::Queue(NetworkResponse{Network::CreateResponse(
							PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, *_ndiff, ndiff),
							HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
						});
					}
					catch (...) {}
				});
			}
			else
				Network::Queue(NetworkResponse{Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(container->GetNetworkID(), ndiff, ContainerDiffNet()),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
				});
		}

		GameFactory::DestroyInstance(_temp);

		result->Unlock(key);
	}
}

void Game::GetRemoveAllItemsEx(const FactoryObject& reference, vector<unsigned char>& data)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	Lockable* result;

#pragma pack(push, 1)
	struct ItemInfo
	{
		unsigned int baseID;
		unsigned int count;
		unsigned int equipped;
		double condition;
	};
#pragma pack(pop)

	ItemInfo* items = reinterpret_cast<ItemInfo*>(&data[0]);
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
		FactoryObject reference = GameFactory::GetObject(PLAYER_REFERENCE);
		cell = vaultcast<Player>(reference)->GetGameCell();
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
		auto& refs = cellRefs[cell][_type];

		set_difference(data.begin(), data.end(), refs.begin(), refs.end(), inserter(diff.first, diff.first.begin()));
		set_difference(refs.begin(), refs.end(), data.begin(), data.end(), inserter(diff.second, diff.second.begin()));

		refs.swap(data);
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
