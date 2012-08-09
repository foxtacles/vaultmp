#include "Game.h"
#include "PacketFactory.h"

unsigned char Game::game = 0x00;
RakNetGUID Game::server;

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
				case Func_CenterOnCell:
				case Func_CenterOnExterior:
				case Func_PlaceAtMe:
				case Func_GetCauseofDeath:
				case Fallout3::Func_CenterOnWorld:
				case FalloutNV::Func_CenterOnWorld:
				case Fallout3::Func_Load:
				case FalloutNV::Func_Load:
					shared = Lockable::Poll(key);
					break;

				default:
					data = Lockable::Retrieve(key);
			}
		}

		FactoryObject reference, self;

		switch (opcode)
		{
			case Func_PlaceAtMe:
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

			case Func_UIMessage:
				break;

			case Fallout3::Func_GetParentCell:
			case FalloutNV::Func_GetParentCell:
			{
				vector<FactoryObject> objects = GameFactory::GetMultiple(vector<unsigned int> {getFrom<double, unsigned int>(info.at(1)), PLAYER_REFERENCE});
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

			case Func_CenterOnCell:
			case Func_CenterOnExterior:
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
			case Func_PlaceAtMe:
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

	Interface::StartDynamic();

	Interface::ExecuteCommand("GetControl", ParamContainer{RawParameter(API::RetrieveAllControls())});
	Interface::ExecuteCommand("DisableControl", ParamContainer{RawParameter(vector<unsigned char>{
		ControlCodes::ControlCode_Quickload,
		ControlCodes::ControlCode_Quicksave,
		ControlCodes::ControlCode_VATS,
		ControlCodes::ControlCode_Rest})});

	Interface::EndDynamic();

	Interface::StartSetup();

	Interface::SetupCommand("GetPos", ParamContainer{self_ref, Object::Param_Axis()});
	Interface::SetupCommand("GetPos", ParamContainer{Player::CreateFunctor(FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE), Object::Param_Axis()}, 30);
	Interface::SetupCommand("GetAngle", ParamContainer{self_ref, RawParameter(vector<string> {API::RetrieveAxis_Reverse(Axis_X), API::RetrieveAxis_Reverse(Axis_Z)})});
	Interface::SetupCommand("GetActorState", ParamContainer{self_ref, Player::CreateFunctor(FLAG_MOVCONTROLS, id)});
	Interface::SetupCommand("GetParentCell", ParamContainer{Player::CreateFunctor(FLAG_ALIVE)}, 30);
	Interface::SetupCommand("ScanContainer", ParamContainer{self_ref}, 50);
	Interface::SetupCommand("GetDead", ParamContainer{Player::CreateFunctor(FLAG_ENABLED | FLAG_ALIVE)}, 30);
	Interface::SetupCommand("GetActorValue", ParamContainer{self_ref, RawParameter(vector<string>{
		API::RetrieveValue_Reverse(ActorVal_Health),
		API::RetrieveValue_Reverse(ActorVal_Head),
		API::RetrieveValue_Reverse(ActorVal_Torso),
		API::RetrieveValue_Reverse(ActorVal_LeftArm),
		API::RetrieveValue_Reverse(ActorVal_RightArm),
		API::RetrieveValue_Reverse(ActorVal_LeftLeg),
		API::RetrieveValue_Reverse(ActorVal_RightLeg)})}, 30);

	// we could exclude health values here
	Interface::SetupCommand("GetActorValue", ParamContainer{self_ref, Actor::Param_ActorValues()}, 100);
	Interface::SetupCommand("GetBaseActorValue", ParamContainer{self_ref, Actor::Param_ActorValues()}, 200);

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

	shared_ptr<Shared<bool>> store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("Load", ParamContainer{RawParameter(savegame)}, key);

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

void Game::CenterOnCell(string cell)
{
	static string last_cell;

	if (cell.empty())
		cell = last_cell;
	else
		last_cell = cell;

	shared_ptr<Shared<bool>> store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("CenterOnCell", ParamContainer{RawParameter(cell)}, key);

	Interface::EndDynamic();

	try
	{
		store.get()->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of cell %s failed (%s)", cell.c_str(), e.what());
	}

	// ready state
}

void Game::CenterOnExterior(signed int x, signed int y)
{
	shared_ptr<Shared<bool>> store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("CenterOnExterior", ParamContainer{RawParameter(x), RawParameter(y)}, key);

	Interface::EndDynamic();

	try
	{
		store.get()->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of cell (%d,%d) failed (%s)", x, y, e.what());
	}

	// ready state
}

void Game::CenterOnWorld(unsigned int baseID, signed int x, signed int y)
{
	shared_ptr<Shared<bool>> store = make_shared<Shared<bool>>();
	unsigned int key = Lockable::Share(store);

	Interface::StartDynamic();

	Interface::ExecuteCommand("CenterOnWorld", ParamContainer{RawParameter(baseID), RawParameter(x), RawParameter(y)}, key);

	Interface::EndDynamic();

	try
	{
		store.get()->get_future(chrono::seconds(60));
	}
	catch (exception& e)
	{
		throw VaultException("Loading of world (%08X,%d,%d) failed (%s)", baseID, x, y, e.what());
	}

	// ready state
}

void Game::LoadEnvironment()
{
	vector<NetworkID> reference = GameFactory::GetIDObjectTypes(ALL_OBJECTS);

	for (NetworkID& id : reference)
	{
		FactoryObject reference = GameFactory::GetObject(id);

		if ((*reference)->GetReference() != PLAYER_REFERENCE)
		{
			if (!(*reference)->IsPersistent())
				(*reference)->SetReference(0x00000000);

			unsigned char type = GameFactory::GetType(*reference);

			switch (type)
			{
				case ID_OBJECT:
					NewObject(reference);
					break;

				case ID_ITEM: // don't create items of containers
					//NewItem(reference);
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
		else
			SetName(reference);
	}
}

void Game::UIMessage(const string& message)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("UIMessage", ParamContainer{RawParameter(message)});

	Interface::EndDynamic();
}

void Game::ChatMessage(const string& message)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("ChatMessage", ParamContainer{RawParameter(message)});

	Interface::EndDynamic();
}

void Game::NewObject(FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	if (!object->GetReference())
	{
		shared_ptr<Shared<unsigned int>> store = make_shared<Shared<unsigned int>>();
		unsigned int key = Lockable::Share(store);

		PlaceAtMe(PLAYER_REFERENCE, object->GetBase(), 1, key);

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
	else
	{
		// existing objects
	}

	SetName(reference);
	//SetPos(reference);
	SetAngle(reference);

	// maybe more
}

void Game::NewItem(FactoryObject& reference)
{
	NewObject(reference);

	// set condition
}

void Game::NewContainer(FactoryObject& reference)
{
	NewObject(reference);
	RemoveAllItems(reference);

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

	SetRestrained(reference, true);

	vector<unsigned char> values = API::RetrieveAllValues();

	for (unsigned char value : values)
	{
		SetActorValue(reference, true, value);
		SetActorValue(reference, false, value);
	}

	Actor* actor = vaultcast<Actor>(reference);

	if (actor->GetActorAlerted())
		SetActorAlerted(reference)();

	if (actor->GetActorSneaking())
		SetActorSneaking(reference)();

	if (actor->GetActorMovingAnimation() != AnimGroup_Idle)
		SetActorMovingAnimation(reference);

	if (actor->GetActorWeaponAnimation() != AnimGroup_Idle)
		SetActorWeaponAnimation(reference);
}

void Game::NewPlayer(FactoryObject& reference)
{
	NewActor(reference);

	// ...

/*
thread t(AsyncTasks<AsyncPack>,
	AsyncPack(async(launch::deferred, [](unsigned int r)
	{
		try
		{
	ParamContainer param_MoveTo{RawParameter(r),RawParameter(PLAYER_REFERENCE) };

	Interface::ExecuteCommand("MoveTo", move(param_MoveTo));
		}
		catch (...) {}
	}, (*reference)->GetReference()), chrono::milliseconds(1000)));
	t.detach();
	*/
}

void Game::RemoveObject(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	if (object->SetEnabled(false))
		ToggleEnabled(reference);

	Interface::StartDynamic();

	Interface::ExecuteCommand("MarkForDelete", ParamContainer{object->GetReferenceParam()});

	Interface::EndDynamic();
}

void Game::PlaceAtMe(const FactoryObject& reference, unsigned int baseID, unsigned int count, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

	PlaceAtMe(container->GetReference(), baseID, count, key);
}

void Game::PlaceAtMe(unsigned int refID, unsigned int baseID, unsigned int count, unsigned int key)
{
	Interface::StartDynamic();

	Interface::ExecuteCommand("PlaceAtMe", ParamContainer{RawParameter(refID), RawParameter(baseID), RawParameter(count)}, key);

	Interface::EndDynamic();
}

void Game::ToggleEnabled(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	Interface::StartDynamic();

	if (object->GetEnabled())
		Interface::ExecuteCommand("Enable", ParamContainer{object->GetReferenceParam(), RawParameter(true)});
	else
		Interface::ExecuteCommand("Disable", ParamContainer{object->GetReferenceParam(), RawParameter(false)});

	Interface::EndDynamic();
}

void Game::Delete(FactoryObject& reference)
{
	RemoveObject(reference);
	GameFactory::DestroyInstance(reference);
}

void Game::SetName(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);
	string name = object->GetName();

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetName", ParamContainer{object->GetReferenceParam(), RawParameter(name)});

	Interface::EndDynamic();
}

void Game::SetRestrained(const FactoryObject& reference, bool restrained)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	//bool restrained = actor->GetActorRestrained();

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetRestrained", ParamContainer{actor->GetReferenceParam(), RawParameter(restrained)});

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

	Interface::ExecuteCommand("SetPos", ParamContainer{object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(object->GetNetworkPos(Axis_X))}, key ? key->Lock() : 0);

	key = object->SetGamePos(Axis_Y, object->GetNetworkPos(Axis_Y));

	Interface::ExecuteCommand("SetPos", ParamContainer{object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Y)), RawParameter(object->GetNetworkPos(Axis_Y))}, key ? key->Lock() : 0);

	key = object->SetGamePos(Axis_Z, object->GetNetworkPos(Axis_Z));

	Interface::ExecuteCommand("SetPos", ParamContainer{object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(object->GetNetworkPos(Axis_Z))}, key ? key->Lock() : 0);

	Interface::EndDynamic();
}

void Game::SetAngle(const FactoryObject& reference)
{
	Object* object = vaultcast<Object>(reference);

	Interface::StartDynamic();

	Interface::ExecuteCommand("SetAngle", ParamContainer{object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_X)), RawParameter(object->GetAngle(Axis_X))});

	double value = object->GetAngle(Axis_Z);
	Actor* actor = vaultcast<Actor>(reference);

	if (actor)
	{
		if (actor->GetActorMovingXY() == 0x01)
			AdjustZAngle(value, -45.0);

		else if (actor->GetActorMovingXY() == 0x02)
			AdjustZAngle(value, 45.0);
	}

	Interface::ExecuteCommand("SetAngle", ParamContainer{object->GetReferenceParam(), RawParameter(API::RetrieveAxis_Reverse(Axis_Z)), RawParameter(value)});

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
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Interface::StartDynamic();

	if (base)
		Interface::ExecuteCommand("SetActorValue", ParamContainer{actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(actor->GetActorBaseValue(index))}, key);
	else
		Interface::ExecuteCommand("ForceActorValue", ParamContainer{actor->GetReferenceParam(), RawParameter(API::RetrieveValue_Reverse(index)), RawParameter(actor->GetActorValue(index))}, key);

	Interface::EndDynamic();
}

function<void()> Game::SetActorSneaking(const FactoryObject& reference, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

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

			Interface::ExecuteCommand("SetForceSneak", ParamContainer{actor->GetReferenceParam(), RawParameter(actor->GetActorSneaking())}, key);

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
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

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

			Interface::ExecuteCommand("SetAlert", ParamContainer{actor->GetReferenceParam(), RawParameter(actor->GetActorAlerted())}, key);

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
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("PlayGroup", ParamContainer{actor->GetReferenceParam(), RawParameter(API::RetrieveAnim_Reverse(anim)), RawParameter(true)}, key);

	Interface::EndDynamic();
}

void Game::SetActorMovingAnimation(const FactoryObject& reference, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	SetActorAnimation(reference, actor->GetActorMovingAnimation(), key);
}

void Game::SetActorWeaponAnimation(const FactoryObject& reference, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	SetActorAnimation(reference, actor->GetActorWeaponAnimation(), key);
}

void Game::KillActor(const FactoryObject& reference, unsigned short limbs, signed char cause, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Interface::StartDynamic();

	// maybe add valid killer later
	if (limbs)
	{
		for (unsigned int i = 1; i <= limbs; i <<= 1)
			if (limbs & i)
				Interface::ExecuteCommand("Kill", ParamContainer{actor->GetReferenceParam(), actor->GetReferenceParam(), RawParameter(static_cast<unsigned int>(i / 2)), RawParameter(cause)}, ((i << 1) > limbs) ? key : 0x00);
	}
	else
		Interface::ExecuteCommand("Kill", ParamContainer{actor->GetReferenceParam(), actor->GetReferenceParam(), RawParameter(Limb_None), RawParameter(cause)}, key);

	Interface::EndDynamic();
}

void Game::FireWeapon(const FactoryObject& reference, unsigned int weapon, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("FireWeapon", ParamContainer{actor->GetReferenceParam(), RawParameter(weapon)}, key);

	Interface::EndDynamic();
}

void Game::AddItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", (*item)->GetReference());

	AddItem(reference, _item->GetBase(), _item->GetItemCount(), _item->GetItemCondition(), _item->GetItemSilent(), key);
}

void Game::AddItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, double condition, bool silent, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("AddItemHealthPercent", ParamContainer{container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(condition / 100), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::RemoveItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", (*item)->GetReference());

	RemoveItem(reference, _item->GetBase(), _item->GetItemCount(), _item->GetItemSilent(), key);
}

void Game::RemoveItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, bool silent, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveItem", ParamContainer{container->GetReferenceParam(), RawParameter(baseID), RawParameter(count), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::RemoveAllItems(const FactoryObject& reference, unsigned int key)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("RemoveAllItems", ParamContainer{container->GetReferenceParam()}, key);

	Interface::EndDynamic();
}

void Game::EquipItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", (*item)->GetReference());

	EquipItem(reference, _item->GetBase(), _item->GetItemSilent(), _item->GetItemStick(), key);
}

void Game::EquipItem(const FactoryObject& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("EquipItem", ParamContainer{actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::UnequipItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key)
{
	Item* _item = vaultcast<Item>(item);

	if (!_item)
		throw VaultException("Object with reference %08X is not an Item", (*item)->GetReference());

	UnequipItem(reference, _item->GetBase(), _item->GetItemSilent(), _item->GetItemStick(), key);
}

void Game::UnequipItem(const FactoryObject& reference, unsigned int baseID, bool silent, bool stick, unsigned int key)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Interface::StartDynamic();

	Interface::ExecuteCommand("UnequipItem", ParamContainer{actor->GetReferenceParam(), RawParameter(baseID), RawParameter(stick), RawParameter(silent)}, key);

	Interface::EndDynamic();
}

void Game::net_SetPos(const FactoryObject& reference, double X, double Y, double Z)
{
	Object* object = vaultcast<Object>(reference);
	bool result = (static_cast<bool>(object->SetNetworkPos(Axis_X, X)) | static_cast<bool>(object->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(object->SetNetworkPos(Axis_Z, Z)));

	if (result && object->GetEnabled())
	{
		Actor* actor = vaultcast<Actor>(reference);   // maybe we should consider items, too (they have physics)

		if (actor == nullptr || (!actor->IsNearPoint(object->GetNetworkPos(Axis_X), object->GetNetworkPos(Axis_Y), object->GetNetworkPos(Axis_Z), 200.0) && actor->GetActorMovingAnimation() == AnimGroup_Idle) || actor->IsActorJumping())
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
		throw VaultException("Object with reference %08X is not a Player", (*player)->GetReference());

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

void Game::net_ContainerUpdate(FactoryObject& reference, const pair<list<NetworkID>, vector<pPacket>>& _diff)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

	Lockable* result;

	// cleaner solution here

	ContainerDiff diff = Container::ToContainerDiff(_diff);
	NetworkID id = container->GetNetworkID();
	GameFactory::LeaveReference(reference);

	while (!(result = container->getLock()));

	reference = GameFactory::GetObject(id);

	unsigned int key = result->Lock();

	GameDiff gamediff = container->ApplyDiff(diff);

	for (const auto& diff : gamediff)
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
}

void Game::net_SetActorValue(const FactoryObject& reference, bool base, unsigned char index, double value)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Lockable* result;

	if (base)
		result = actor->SetActorBaseValue(index, value);
	else
		result = actor->SetActorValue(index, value);

	if (result)
		SetActorValue(reference, base, index, result->Lock());
}

void Game::net_SetActorState(const FactoryObject& reference, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

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
		weapon != AnimGroup_AttackLeftIS && weapon != AnimGroup_AttackRightIS && (weapon != AnimGroup_Aim || prev_weapon == AnimGroup_AimIS))
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
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

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
			GameFactory::LeaveReference(reference);
			//Game::LoadGame();
			Game::LoadEnvironment();

			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, false, 0, 0),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
			});
		}
	}
}

void Game::net_FireWeapon(const FactoryObject& reference, unsigned int weapon)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	FireWeapon(reference, weapon);
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
		throw VaultException("Object with reference %08X is not a Player", (*player)->GetReference());

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
		Network::Queue(NetworkResponse{Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(object->GetNetworkID(), cell),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, server)
		});
}

void Game::GetDead(const FactoryObject& reference, const FactoryObject& player, bool dead)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	Player* self = vaultcast<Player>(player);

	if (!self)
		throw VaultException("Object with reference %08X is not a Player", (*player)->GetReference());

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
						shared_ptr<Shared<unsigned short>> store = make_shared<Shared<unsigned short>>();
						key = Lockable::Share(store);

						Interface::StartDynamic();

						Interface::ExecuteCommand("IsLimbGone", ParamContainer{RawParameter(refID), RawParameter(vector<unsigned char>{
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
						shared_ptr<Shared<signed char>> store = make_shared<Shared<signed char>>();
						key = Lockable::Share(store);

						Interface::StartDynamic();

						Interface::ExecuteCommand("GetCauseofDeath", ParamContainer{RawParameter(refID)}, key);

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
	shared_ptr<Lockable> shared = Lockable::Poll(key, limb == Limb_Weapon).lock();
	Lockable* locked = shared.get();

	if (locked == nullptr)
		throw VaultException("Storage has expired");

	Shared<unsigned short>* store = dynamic_cast<Shared<unsigned short>*>(locked);

	if (store == nullptr)
		throw VaultException("Storage is corrupted");

	store->set(store->get() | (static_cast<unsigned short>(gone) << limb));

	if (limb == Limb_Weapon)
		store->set_promise();
}

void Game::GetActorValue(const FactoryObject& reference, bool base, unsigned char index, double value)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

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

void Game::GetActorState(const FactoryObject& reference, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool sneaking)
{
	Actor* actor  = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

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
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), moving, movingxy, weapon, actor->GetActorAlerted(), sneaking),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
		});
}

void Game::GetControl(const FactoryObject& reference, unsigned char control, unsigned char key)
{
	Player* player = vaultcast<Player>(reference);

	if (!player)
		throw VaultException("Object with reference %08X is not a Player", (*reference)->GetReference());

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
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

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
			Network::Queue(NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(container->GetNetworkID(), Container::ToNetDiff(diff)),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, server)
			});

			container->ApplyDiff(diff);
		}

		GameFactory::DestroyInstance(_temp);

		result->Unlock(key);
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
