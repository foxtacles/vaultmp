#include "Game.h"

unsigned char Game::game = 0x00;
RakNetGUID Game::server;

#ifdef VAULTMP_DEBUG
Debug* Game::debug = NULL;
#endif

using namespace Values;

#ifdef VAULTMP_DEBUG
void Game::SetDebugHandler(Debug* debug)
{
    Game::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Game class", true);
}
#endif

void Game::Initialize()
{
    Interface::DefineCommand("GetPos", "%0.GetPos %1");
    Interface::DefineCommand("GetPosNotSelf", "%0.GetPos %1", "GetPos");
    Interface::DefineCommand("SetPos", "%0.SetPos %1 %2");
    Interface::DefineCommand("GetAngle", "%0.GetAngle %1");
    Interface::DefineCommand("SetAngle", "%0.SetAngle %1 %2");
    Interface::DefineCommand("GetParentCell", "%0.GetParentCell");
    Interface::DefineCommand("GetBaseActorValue", "%0.GetBaseActorValue %1");
    Interface::DefineCommand("SetActorValue", "%0.SetActorValue %1 %2");
    Interface::DefineCommand("ForceActorValue", "%0.ForceActorValue %1 %2");
    Interface::DefineCommand("GetActorValue", "%0.GetActorValue %1");
    Interface::DefineCommand("GetActorValueHealth", "%0.GetActorValue %1", "GetActorValue");
    Interface::DefineCommand("GetDead", "%0.GetDead");
    Interface::DefineCommand("Enable", "%0.Enable %1");
    Interface::DefineCommand("Disable", "%0.Disable %1");
    Interface::DefineCommand("MoveTo", "%0.MoveTo %1 %2 %3 %4");
    Interface::DefineCommand("SetRestrained", "%0.SetRestrained %1");
    Interface::DefineCommand("PlayGroup", "%0.PlayGroup %1 %2");
    Interface::DefineCommand("SetAlert", "%0.SetAlert %1");
    Interface::DefineCommand("SetName", "%0.SetName %1");
    Interface::DefineCommand("EquipItem", "%0.EquipItem %1 %2 %3");
    Interface::DefineCommand("UnequipItem", "%0.UnequipItem %1 %2 %3");
    Interface::DefineCommand("AddItem", "%0.AddItem %1 %2 %3");
    Interface::DefineCommand("RemoveItem", "%0.RemoveItem %1 %2 %3");
    Interface::DefineCommand("RemoveAllItems", "%0.RemoveAllItems");
    Interface::DefineCommand("MoveAllItems", "%0.RemoveAllItems %1 %2", "RemoveAllItems");
    Interface::DefineCommand("KillActor", "%0.Kill %1 %2 %3", "Kill");
    Interface::DefineCommand("Kill", "%0.Kill");
    Interface::DefineCommand("PlaceAtMe", "%0.PlaceAtMe %1 %2 %3 %4");
    Interface::DefineCommand("Load", "Load %0");

    if (game & FALLOUT_GAMES)
    {
        Interface::DefineCommand("IsMoving", "%0.IsMoving");
        Interface::DefineCommand("IsAnimPlaying", "%0.IsAnimPlaying %1");
        Interface::DefineCommand("MarkForDelete", "%0.MarkForDelete");
    }
    else
        Interface::DefineCommand("IsAnimGroupPlaying", "%0.IsAnimGroupPlaying %1");

    Interface::DefineCommand("GetActorState", "%0.GetActorState");

    Player* self = (Player*) GameFactory::GetObject(ID_PLAYER, PLAYER_REFERENCE);

    Interface::StartSession();

    try
    {
        ParamList param_GetPos;
        param_GetPos.push_back(self->GetReferenceParam());
        param_GetPos.push_back(Object::Param_Axis());
        ParamContainer GetPos = ParamContainer(param_GetPos, &Data::AlwaysTrue);
        Interface::DefineNative("GetPos", GetPos);
        Interface::ExecuteCommandLoop("GetPos");

        ParamList param_GetPos_NotSelf;
        param_GetPos_NotSelf.push_back(Player::CreateFunctor(FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE));
        param_GetPos_NotSelf.push_back(Object::Param_Axis());
        ParamContainer GetPos_NotSelf = ParamContainer(param_GetPos_NotSelf, &Data::AlwaysTrue);
        Interface::DefineNative("GetPosNotSelf", GetPos_NotSelf);
        Interface::ExecuteCommandLoop("GetPosNotSelf", 30);

        ParamList param_GetAngle;
        param_GetAngle.push_back(self->GetReferenceParam());
        param_GetAngle.push_back(BuildParameter(vector<string> {API::RetrieveAxis_Reverse(Axis_X), API::RetrieveAxis_Reverse(Axis_Z)})); // exclude Y-angle, not used
        ParamContainer GetAngle = ParamContainer(param_GetAngle, &Data::AlwaysTrue);
        Interface::DefineNative("GetAngle", GetAngle);
        Interface::ExecuteCommandLoop("GetAngle");

        ParamList param_GetActorState;
        param_GetActorState.push_back(self->GetReferenceParam());
        ParamContainer GetActorState = ParamContainer(param_GetActorState, &Data::AlwaysTrue);
        Interface::DefineNative("GetActorState", GetActorState);
        Interface::ExecuteCommandLoop("GetActorState");

        ParamList param_GetParentCell;
        param_GetParentCell.push_back(Player::CreateFunctor(0x00000000));
        ParamContainer GetParentCell = ParamContainer(param_GetParentCell, &Data::AlwaysTrue);
        Interface::DefineNative("GetParentCell", GetParentCell);
        Interface::ExecuteCommandLoop("GetParentCell", 30);

        ParamList param_GetDead;
        param_GetDead.push_back(Player::CreateFunctor(FLAG_ENABLED));
        ParamContainer GetDead = ParamContainer(param_GetDead, &Data::AlwaysTrue);
        Interface::DefineNative("GetDead", GetDead);
        Interface::ExecuteCommandLoop("GetDead", 30);

        vector<string> healthValues;
        if (game & FALLOUT_GAMES)
        {
            healthValues.push_back(API::RetrieveValue_Reverse(Fallout::ActorVal_Health));
            healthValues.push_back(API::RetrieveValue_Reverse(Fallout::ActorVal_Head));
            healthValues.push_back(API::RetrieveValue_Reverse(Fallout::ActorVal_Torso));
            healthValues.push_back(API::RetrieveValue_Reverse(Fallout::ActorVal_LeftArm));
            healthValues.push_back(API::RetrieveValue_Reverse(Fallout::ActorVal_RightArm));
            healthValues.push_back(API::RetrieveValue_Reverse(Fallout::ActorVal_LeftLeg));
            healthValues.push_back(API::RetrieveValue_Reverse(Fallout::ActorVal_RightLeg));
        }
        else
        {

        }

        ParamList param_GetActorValue_Health;
        //param_GetActorValue_Health.push_back(Player::Param_EnabledPlayers);
        param_GetActorValue_Health.push_back(self->GetReferenceParam());
        param_GetActorValue_Health.push_back(BuildParameter(healthValues));
        ParamContainer GetActorValue_Health = ParamContainer(param_GetActorValue_Health, &Data::AlwaysTrue);
        Interface::DefineNative("GetActorValueHealth", GetActorValue_Health);
        Interface::ExecuteCommandLoop("GetActorValueHealth", 30);

        ParamList param_GetActorValue;
        param_GetActorValue.push_back(self->GetReferenceParam());
        param_GetActorValue.push_back(Actor::Param_ActorValues()); // we could exclude health values here
        ParamContainer GetActorValue = ParamContainer(param_GetActorValue, &Data::AlwaysTrue);
        Interface::DefineNative("GetActorValue", GetActorValue);
        Interface::ExecuteCommandLoop("GetActorValue", 100);

        ParamList param_GetBaseActorValue;
        param_GetBaseActorValue.push_back(self->GetReferenceParam());
        param_GetBaseActorValue.push_back(Actor::Param_ActorValues());
        ParamContainer GetBaseActorValue = ParamContainer(param_GetBaseActorValue, &Data::AlwaysTrue);
        Interface::DefineNative("GetBaseActorValue", GetBaseActorValue);
        Interface::ExecuteCommandLoop("GetBaseActorValue", 200);
    }
    catch (...)
    {
        Interface::EndSession();
        GameFactory::LeaveReference(self);
        throw;
    }

    Interface::EndSession();
    GameFactory::LeaveReference(self);
}

void Game::LoadGame(string savegame)
{
    Utils::RemoveExtension(savegame);

    Interface::StartSession();

    ParamList param_Load;
    param_Load.push_back(BuildParameter(savegame));
    ParamContainer Load = ParamContainer(param_Load, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("Load", Load);

    Interface::EndSession();
}

void Game::NewPlayer(NetworkID id, unsigned int baseID, string name)
{
    Player* self = (Player*) GameFactory::GetObject(ID_PLAYER, PLAYER_REFERENCE);

    Value<unsigned int>* store = new Value<unsigned int>;
    signed int key = store->Lock(true);

    Interface::StartSession();

    ParamList param_PlaceAtMe;
    param_PlaceAtMe.push_back(self->GetReferenceParam()); // need something else here
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(baseID)));
    param_PlaceAtMe.push_back(Data::Param_True);
    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 2, key);

    Interface::EndSession();

    GameFactory::LeaveReference(self);

    for (int i = 0; i < 100 && !store->Get(); i++)
        Sleep(100);

    if (!store->Get())
    {
        delete store;
        throw VaultException("Player creation with baseID %08X and NetworkID %lld failed", baseID, id);
    }

    GameFactory::CreateKnownInstance(ID_PLAYER, id, baseID);
    Player* player = (Player*) GameFactory::GetObject(ID_PLAYER, id);
    player->SetReference(store->Get());
    delete store;

    SetName(id, name);
    SetRestrained(id, true);

    GameFactory::LeaveReference(player);
}

void Game::PlayerLeft(NetworkID id)
{
    Player* player = (Player*) GameFactory::GetObject(ID_PLAYER, id);
    // Delete invokes DestroyInstance, we must make sure that the lock count is zero here, this function performs only a type check
    GameFactory::LeaveReference(player);
    Delete(id);
}

void Game::Enable(NetworkID id, bool enable)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, id);
    bool result = (bool) object->SetEnabled(enable);

    if (result)
    {
        Interface::StartSession();

        if (enable)
        {
            ParamList param_Enable;
            param_Enable.push_back(object->GetReferenceParam());
            param_Enable.push_back(Data::Param_True);
            ParamContainer Enable = ParamContainer(param_Enable, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("Enable", Enable);
        }
        else
        {
            ParamList param_Disable;
            param_Disable.push_back(object->GetReferenceParam());
            param_Disable.push_back(Data::Param_False);
            ParamContainer Disable = ParamContainer(param_Disable, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("Disable", Disable);
        }

        Interface::EndSession();
    }

    GameFactory::LeaveReference(object);
}

void Game::Delete(NetworkID id)
{
    Enable(id, false);
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, id);

    if (game & FALLOUT_GAMES)
    {
        Interface::StartSession();

        ParamList param_MarkForDelete;
        param_MarkForDelete.push_back(object->GetReferenceParam());
        ParamContainer MarkForDelete = ParamContainer(param_MarkForDelete, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("MarkForDelete", MarkForDelete);

        Interface::EndSession();
    }

    GameFactory::DestroyInstance(object);
}

void Game::SetName(NetworkID id, string name)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, id);
    object->SetName(name);

    SetName(object->GetReference());

    GameFactory::LeaveReference(object);
}

void Game::SetName(unsigned int refID)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, refID);

    Interface::StartSession();

    ParamList param_SetName;
    param_SetName.push_back(object->GetReferenceParam());
    param_SetName.push_back(BuildParameter(object->GetName()));
    ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("SetName", SetName);

    Interface::EndSession();

    GameFactory::LeaveReference(object);
}

void Game::SetRestrained(NetworkID id, bool restrained, unsigned int delay)
{
    Actor* actor = (Actor*) GameFactory::GetObject(ALL_ACTORS, id);

    Interface::StartSession();

    ParamList param_SetRestrained;
    param_SetRestrained.push_back(actor->GetReferenceParam());
    param_SetRestrained.push_back(restrained ? Data::Param_True : Data::Param_False);
    ParamContainer SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("SetRestrained", SetRestrained, 0, delay);

    Interface::EndSession();

    GameFactory::LeaveReference(actor);
}

void Game::SetPos(NetworkID id, unsigned char axis, double value)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, id);
    double old_value = object->GetPos(axis);
    Lockable* result = object->SetPos(axis, value);

    if (result && object->GetEnabled())
    {
        Actor* actor = NULL; // maybe we should consider items, too (they have physics)
        if (GameFactory::GetType(object) & ALL_ACTORS)
            actor = (Actor*) object;

        if (actor == NULL || !actor->IsCoordinateInRange(axis, old_value, 150.0))
        {
            signed int key = result->Lock(true);

            Interface::StartSession();

            ParamList param_SetPos;
            param_SetPos.push_back(object->GetReferenceParam());
            param_SetPos.push_back(BuildParameter(API::RetrieveAxis_Reverse(axis)));
            param_SetPos.push_back(BuildParameter(value));
            ParamContainer SetPos = ParamContainer(param_SetPos, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetPos", SetPos, 0, 2, key);

            Interface::EndSession();
        }
    }

    GameFactory::LeaveReference(object);
}

void Game::SetAngle(NetworkID id, unsigned char axis, double value)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, id);
    bool result = (bool) object->SetAngle(axis, value);

    if (result && object->GetEnabled())
        SetAngle(object->GetReference(), axis);

    GameFactory::LeaveReference(object);
}

void Game::SetAngle(unsigned int refID, unsigned char axis)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, refID);

    Interface::StartSession();

    ParamList param_SetAngle;
    param_SetAngle.push_back(object->GetReferenceParam());
    param_SetAngle.push_back(BuildParameter(API::RetrieveAxis_Reverse(axis)));
    param_SetAngle.push_back(BuildParameter(object->GetAngle(axis)));
    ParamContainer SetAngle = ParamContainer(param_SetAngle, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("SetAngle", SetAngle);

    Interface::EndSession();

    GameFactory::LeaveReference(object);
}

void Game::SetNetworkCell(NetworkID id, unsigned int cell)
{
    vector<Reference*> objects = GameFactory::GetMultiple(vector<ObjectNetwork> {ObjectNetwork(ALL_OBJECTS, id), ObjectNetwork(ID_PLAYER, GameFactory::LookupNetworkID(PLAYER_REFERENCE))});
    Object* object = (Object*) objects[0];
    Player* self = (Player*) objects[1];

    object->SetNetworkCell(cell);

    if (object != self)
    {
        if (object->GetNetworkCell() != self->GetGameCell())
            Enable(object->GetNetworkID(), false);
        else
            Enable(object->GetNetworkID(), true);
    }

    GameFactory::LeaveReference(objects);
}

void Game::SetActorValue(NetworkID id, bool base, unsigned char index, double value)
{
    Actor* actor = (Actor*) GameFactory::GetObject(ALL_ACTORS, id);

    Lockable* result;

    try
    {
        if (base)
            result = actor->SetActorBaseValue(index, value);
        else
            result = actor->SetActorValue(index, value);
    }
    catch (...)
    {
        GameFactory::LeaveReference(actor);
        throw;
    }

    if (result)
    {
        signed int key = result->Lock(true);

        Interface::StartSession();

        if (base)
        {
            ParamList param_SetActorValue;
            param_SetActorValue.push_back(actor->GetReferenceParam());
            param_SetActorValue.push_back(BuildParameter(API::RetrieveValue_Reverse(index)));
            param_SetActorValue.push_back(BuildParameter(value));
            ParamContainer SetActorValue = ParamContainer(param_SetActorValue, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetActorValue", SetActorValue, 0, 2, key);
        }
        else
        {
            ParamList param_ForceActorValue;
            param_ForceActorValue.push_back(actor->GetReferenceParam());
            param_ForceActorValue.push_back(BuildParameter(API::RetrieveValue_Reverse(index)));
            param_ForceActorValue.push_back(BuildParameter(value));
            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue, 0, 2, key);
        }

        Interface::EndSession();
    }

    GameFactory::LeaveReference(actor);
}

void Game::SetActorState(NetworkID id, unsigned char index, bool alerted)
{
    Actor* actor = (Actor*) GameFactory::GetObject(ALL_ACTORS, id);

    Lockable* result;

    try
    {
        result = actor->SetActorRunningAnimation(index);
    }
    catch (...)
    {
        GameFactory::LeaveReference(actor);
        throw;
    }

    if (result && actor->GetEnabled())
    {
        signed int key = result->Lock(true);

        Interface::StartSession();

        ParamList param_PlayGroup;
        param_PlayGroup.push_back(actor->GetReferenceParam());
        param_PlayGroup.push_back(BuildParameter(API::RetrieveAnim_Reverse(index)));
        param_PlayGroup.push_back(Data::Param_True);
        ParamContainer PlayGroup = ParamContainer(param_PlayGroup, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("PlayGroup", PlayGroup, 0, 2, key);

        Interface::EndSession();
    }

    result = actor->SetActorAlerted(alerted);

    if (result && actor->GetEnabled())
    {
        SetRestrained(id, false);

        signed int key = result->Lock(true);

        Interface::StartSession();

        ParamList param_SetAlert;
        param_SetAlert.push_back(actor->GetReferenceParam());
        param_SetAlert.push_back(alerted ? Data::Param_True : Data::Param_False);
        ParamContainer SetAlert = ParamContainer(param_SetAlert, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("SetAlert", SetAlert, 0, 50, key);

        Interface::EndSession();

        SetRestrained(id, true, 2000); // prevents more or less efficiently alert state desync
    }

    GameFactory::LeaveReference(actor);
}

void Game::MoveTo(NetworkID id, NetworkID id2, bool cell)
{
    vector<Reference*> objects = GameFactory::GetMultiple(vector<ObjectNetwork> {ObjectNetwork(ALL_OBJECTS, id), ObjectNetwork(ALL_OBJECTS, id2)});
    Object* object = (Object*) objects[0];
    Object* object2 = (Object*) objects[1];

    Lockable* result = object->SetGameCell(object2->GetGameCell());

    if (result)
    {
        signed int key = result->Lock(true);

        Interface::StartSession();

        ParamList param_MoveTo;
        param_MoveTo.push_back(object->GetReferenceParam());
        param_MoveTo.push_back(object2->GetReferenceParam());

        if (cell)
        {
            param_MoveTo.push_back(BuildParameter(object->GetPos(Axis_X) - object2->GetPos(Axis_X)));
            param_MoveTo.push_back(BuildParameter(object->GetPos(Axis_Y) - object2->GetPos(Axis_Y)));
            param_MoveTo.push_back(BuildParameter(object->GetPos(Axis_Z) - object2->GetPos(Axis_Z)));
        }
        else
        {
            param_MoveTo.push_back(Data::Param_False);
            param_MoveTo.push_back(Data::Param_False);
            param_MoveTo.push_back(Data::Param_False);
        }

        ParamContainer MoveTo = ParamContainer(param_MoveTo, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("MoveTo", MoveTo, 0, 2, key);

        Interface::EndSession();
    }

    GameFactory::LeaveReference(objects);
}

NetworkResponse Game::Authenticate(string password)
{
    NetworkResponse response;
    Player* self = (Player*) GameFactory::GetObject(ID_PLAYER, PLAYER_REFERENCE);
    pDefault* packet = PacketFactory::CreatePacket(ID_GAME_AUTH, self->GetName().c_str(), password.c_str());
    response = Network::CompleteResponse(Network::CreateResponse(packet,
                                         (unsigned char) HIGH_PRIORITY,
                                         (unsigned char) RELIABLE_ORDERED,
                                         CHANNEL_GAME,
                                         server));
    GameFactory::LeaveReference(self);
    return response;
}

void Game::PlaceAtMe(Lockable* data, unsigned int refID)
{
    if (data == NULL)
        throw VaultException("Could not relocate reference storage");

    Value<unsigned int>* store = dynamic_cast<Value<unsigned int>*>(data);

    if (store == NULL)
        throw VaultException("Reference storage is corrupted");

    store->Set(refID);
}

void Game::GetPos(unsigned int refID, unsigned char axis, double value)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, refID);
    bool result = (bool) object->SetPos(axis, value);

    if (result && object->GetReference() == PLAYER_REFERENCE)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_POS, object->GetNetworkID(), axis, value);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_SEQUENCED,
                                   CHANNEL_GAME,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(object);
}

void Game::GetAngle(unsigned int refID, unsigned char axis, double value)
{
    Object* object = (Object*) GameFactory::GetObject(ALL_OBJECTS, refID);
    bool result = (bool) object->SetAngle(axis, value);

    if (result)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_ANGLE, object->GetNetworkID(), axis, value);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_SEQUENCED,
                                   CHANNEL_GAME,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(object);
}

void Game::GetParentCell(unsigned int refID, unsigned int cell)
{
    vector<Reference*> objects = GameFactory::GetMultiple(vector<ObjectReference> {ObjectReference(ALL_OBJECTS, refID), ObjectReference(ID_PLAYER, PLAYER_REFERENCE)});
    Object* object = (Object*) objects[0];
    Player* self = (Player*) objects[1];

    if (object != self)
    {
        if (self->GetGameCell() == object->GetNetworkCell() && object->GetGameCell() != object->GetNetworkCell())
        {
            Enable(object->GetNetworkID(), true);
            MoveTo(object->GetNetworkID(), self->GetNetworkID(), true);
            SetAngle(object->GetNetworkID(), Axis_Z);
        }
    }

    bool result = (bool) object->SetGameCell(cell);

    if (object != self)
    {
        if (object->GetNetworkCell() != self->GetGameCell())
            Enable(object->GetNetworkID(), false);
        else
            Enable(object->GetNetworkID(), true);
    }

    if (result && object == self)
    {

        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_CELL, object->GetNetworkID(), cell);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_SEQUENCED,
                                   CHANNEL_GAME,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(objects);
}

void Game::GetActorValue(unsigned int refID, bool base, unsigned char index, double value)
{
    Actor* actor = (Actor*) GameFactory::GetObject(ALL_ACTORS, refID);
    bool result;

    try
    {
        if (base)
            result = (bool) actor->SetActorBaseValue(index, value);
        else
            result = (bool) actor->SetActorValue(index, value);
    }
    catch (...)
    {
        GameFactory::LeaveReference(actor);
        throw;
    }

    if (result)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_VALUE, actor->GetNetworkID(), base, index, value);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_ORDERED,
                                   CHANNEL_GAME,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(actor);
}

void Game::GetActorState(unsigned int refID, unsigned char index, bool alerted)
{
    Actor* actor = (Actor*) GameFactory::GetObject(ALL_ACTORS, refID);
    bool result;

    if (index == 0xFF)
        index = AnimGroup_Idle;

    try
    {
        result = ((bool) actor->SetActorRunningAnimation(index) | (bool) actor->SetActorAlerted(alerted));
    }
    catch (...)
    {
        GameFactory::LeaveReference(actor);
        throw;
    }

    if (result)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_STATE, actor->GetNetworkID(), index, alerted);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_ORDERED,
                                   CHANNEL_GAME,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(actor);
}

void Game::Failure_PlaceAtMe(unsigned int refID, unsigned int baseID, unsigned int count, signed int key)
{
    Interface::StartSession();

    ParamList param_PlaceAtMe;
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(refID)));
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(baseID)));
    param_PlaceAtMe.push_back(BuildParameter(count));
    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 2, key);

    Interface::EndSession();
}
