#include "Game.h"

int Game::game = 0;
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
    Interface::DefineCommand("IsMoving", "%0.IsMoving");
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
    Interface::DefineCommand("MarkForDelete", "%0.MarkForDelete");
    Interface::DefineCommand("Load", "Load %0");

    Player* self = (Player*) GameFactory::GetObject(PLAYER_REFERENCE);

    Interface::StartSession();

    ParamList param_GetPos;
    param_GetPos.push_back(self->GetReferenceParam());
    param_GetPos.push_back(Actor::Param_Axis);
    ParamContainer GetPos = ParamContainer(param_GetPos, &Data::AlwaysTrue);
    Interface::DefineNative("GetPos", GetPos);
    Interface::ExecuteCommandLoop("GetPos");

    ParamList param_GetPos_NotSelf;
    param_GetPos_NotSelf.push_back(Player::Param_EnabledPlayers_NotSelf);
    param_GetPos_NotSelf.push_back(Actor::Param_Axis);
    ParamContainer GetPos_NotSelf = ParamContainer(param_GetPos_NotSelf, &Data::AlwaysTrue);
    Interface::DefineNative("GetPosNotSelf", GetPos_NotSelf);
    Interface::ExecuteCommandLoop("GetPosNotSelf", 30);

    ParamList param_GetAngle;
    param_GetAngle.push_back(self->GetReferenceParam());
    param_GetAngle.push_back(BuildParameter(vector<string>{API::RetrieveAxis_Reverse(Axis_X), API::RetrieveAxis_Reverse(Axis_Z)})); // exclude Y-angle, not used
    ParamContainer GetAngle = ParamContainer(param_GetAngle, &Data::AlwaysTrue);
    Interface::DefineNative("GetAngle", GetAngle);
    Interface::ExecuteCommandLoop("GetAngle");

    ParamList param_GetParentCell;
    param_GetParentCell.push_back(Player::Param_AllPlayers);
    ParamContainer GetParentCell = ParamContainer(param_GetParentCell, &Data::AlwaysTrue);
    Interface::DefineNative("GetParentCell", GetParentCell);
    Interface::ExecuteCommandLoop("GetParentCell", 30);

    ParamList param_GetDead;
    param_GetDead.push_back(Player::Param_EnabledPlayers);
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
    param_GetActorValue_Health.push_back(Player::Param_EnabledPlayers);
    param_GetActorValue_Health.push_back(BuildParameter(healthValues));
    ParamContainer GetActorValue_Health = ParamContainer(param_GetActorValue_Health, &Data::AlwaysTrue);
    Interface::DefineNative("GetActorValueHealth", GetActorValue_Health);
    Interface::ExecuteCommandLoop("GetActorValueHealth", 30);

    ParamList param_GetActorValue;
    param_GetActorValue.push_back(self->GetReferenceParam());
    param_GetActorValue.push_back(Actor::Param_ActorValues); // we could exclude health values here
    ParamContainer GetActorValue = ParamContainer(param_GetActorValue, &Data::AlwaysTrue);
    Interface::DefineNative("GetActorValue", GetActorValue);
    Interface::ExecuteCommandLoop("GetActorValue", 100);

    ParamList param_GetBaseActorValue;
    param_GetBaseActorValue.push_back(self->GetReferenceParam());
    param_GetBaseActorValue.push_back(Actor::Param_ActorValues);
    ParamContainer GetBaseActorValue = ParamContainer(param_GetBaseActorValue, &Data::AlwaysTrue);
    Interface::DefineNative("GetBaseActorValue", GetBaseActorValue);
    Interface::ExecuteCommandLoop("GetBaseActorValue", 200);

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
    Player* player = (Player*) GameFactory::CreateInstance(ID_PLAYER, baseID);
    player->SetNetworkID(id);
    player->SetName(name);

    Value<Object*>* store = new Value<Object*>;
    store->Set(player);
    signed int key = store->Lock(true);

    Player* self = (Player*) GameFactory::GetObject(PLAYER_REFERENCE);

    Interface::StartSession();

    ParamList param_PlaceAtMe;
    param_PlaceAtMe.push_back(self->GetReferenceParam()); // need something else here
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(baseID)));
    param_PlaceAtMe.push_back(Data::Param_True);
    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 2, key);

    Interface::EndSession();

    GameFactory::LeaveReference(self);

    for (int i = 0; i < 30 && !player->GetReference(); i++)
        Sleep(100);

    if (!player->GetReference())
    {
        GameFactory::DestroyInstance(player);
        throw VaultException("Player creation with baseID %08X and NetworkID %lld failed", baseID, id);
    }

    SetName(id, name);
    SetRestrained(id, true);

    GameFactory::LeaveReference(player);
}

void Game::PlayerLeft(NetworkID id)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    Player* player = dynamic_cast<Player*>(object);

    if (player == NULL)
    {
        GameFactory::LeaveReference(object);
        throw VaultException("Object %lld is not a Player", id);
    }

    Delete(id);

    GameFactory::LeaveReference(player);
}

void Game::Enable(NetworkID id, bool enable)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

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
    Game::Enable(id, false);

    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    if (game & FALLOUT_GAMES)
    {
        Interface::StartSession();

        ParamList param_MarkForDelete;
        param_MarkForDelete.push_back(object->GetReferenceParam());
        ParamContainer MarkForDelete = ParamContainer(param_MarkForDelete, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("MarkForDelete", MarkForDelete);

        Interface::EndSession();
    }

    GameFactory::DestroyInstance(id);
}

void Game::SetName(NetworkID id, string name)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    object->SetName(name);

    Interface::StartSession();

    ParamList param_SetName;
    param_SetName.push_back(object->GetReferenceParam());
    param_SetName.push_back(BuildParameter(name));
    ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("SetName", SetName);

    Interface::EndSession();

    GameFactory::LeaveReference(object);
}

void Game::SetName(unsigned int refID)
{
    Object* object = (Object*) GameFactory::GetObject(refID);

    if (object == NULL)
        throw VaultException("Unknown object reference %08X", refID);

    SetName(object->GetNetworkID(), object->GetName());

    GameFactory::LeaveReference(object);
}

void Game::SetRestrained(NetworkID id, bool restrained)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    Actor* actor = dynamic_cast<Actor*>(object);

    if (actor == NULL)
    {
        GameFactory::LeaveReference(object);
        throw VaultException("SetRestrained is only applicable on actors");
    }

    Interface::StartSession();

    ParamList param_SetRestrained;
    param_SetRestrained.push_back(actor->GetReferenceParam());
    param_SetRestrained.push_back(restrained ? Data::Param_True : Data::Param_False);
    ParamContainer SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("SetRestrained", SetRestrained);

    Interface::EndSession();

    GameFactory::LeaveReference(actor);
}

void Game::SetPos(NetworkID id, unsigned char axis, double value)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    double old_value = object->GetPos(axis);
    Lockable* result = object->SetPos(axis, value);

    if (result && object->GetEnabled())
    {
        Actor* actor = dynamic_cast<Actor*>(object); // maybe we should consider items, too (they have physics)

        if (actor == NULL || !actor->IsCoordinateInRange(axis, old_value, 300.0))
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
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    bool result = (bool) object->SetAngle(axis, value);

    if (result && object->GetEnabled())
    {
        Interface::StartSession();

        ParamList param_SetAngle;
        param_SetAngle.push_back(object->GetReferenceParam());
        param_SetAngle.push_back(BuildParameter(API::RetrieveAxis_Reverse(axis)));
        param_SetAngle.push_back(BuildParameter(value));
        ParamContainer SetAngle = ParamContainer(param_SetAngle, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("SetAngle", SetAngle);

        Interface::EndSession();
    }

    GameFactory::LeaveReference(object);
}

void Game::SetNetworkCell(NetworkID id, unsigned int cell)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    object->SetNetworkCell(cell);

    GameFactory::LeaveReference(object);
}

void Game::SetActorValue(NetworkID id, bool base, unsigned char index, double value)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    Actor* actor = dynamic_cast<Actor*>(object);

    if (actor == NULL)
    {
        GameFactory::LeaveReference(object);
        throw VaultException("Object %lld is not an Actor", id);
    }

    Lockable* result;

    if (base)
        result = actor->SetActorBaseValue(index, value);
    else
        result = actor->SetActorValue(index, value);

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

void Game::MoveTo(NetworkID id, NetworkID id2)
{
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    Object* object2 = (Object*) GameFactory::GetObject(id2);

    if (object2 == NULL)
    {
        GameFactory::LeaveReference(object);
        throw VaultException("Unknown object with NetworkID %lld", id2);
    }

    Lockable* result = object->SetGameCell(object2->GetGameCell());

    if (result)
    {
        signed int key = result->Lock(true);

        Interface::StartSession();

        ParamList param_MoveTo;
        param_MoveTo.push_back(object->GetReferenceParam());
        param_MoveTo.push_back(object2->GetReferenceParam());
        param_MoveTo.push_back(Data::Param_False);
        param_MoveTo.push_back(Data::Param_False);
        param_MoveTo.push_back(Data::Param_False);
        ParamContainer MoveTo = ParamContainer(param_MoveTo, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("MoveTo", MoveTo, 0, 2, key);

        Interface::EndSession();
    }

    GameFactory::LeaveReference(object);
    GameFactory::LeaveReference(object2);
}

NetworkResponse Game::Authenticate(string password)
{
    NetworkResponse response;
    Player* self = (Player*) GameFactory::GetObject(PLAYER_REFERENCE);
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
        throw VaultException("Could not relocate object pointer storage");

    Value<Object*>* store = dynamic_cast<Value<Object*>*>(data);

    if (store == NULL)
        throw VaultException("Object pointer storage is corrupted");

    store->Get()->SetReference(refID);

    delete store;
}

void Game::GetPos(unsigned int refID, unsigned char axis, double value)
{
    Object* object = (Object*) GameFactory::GetObject(refID);

    if (object == NULL)
        throw VaultException("Unknown object with reference %08X", refID);

    bool result = (bool) object->SetPos(axis, value);

    Player* self = (Player*) GameFactory::GetObject(PLAYER_REFERENCE);

    if (result && object == self)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_POS, object->GetNetworkID(), axis, value);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_SEQUENCED,
                                   CHANNEL_DATA,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(self);
    GameFactory::LeaveReference(object);
}

void Game::GetAngle(unsigned int refID, unsigned char axis, double value)
{
    Object* object = (Object*) GameFactory::GetObject(refID);

    if (object == NULL)
        throw VaultException("Unknown object with reference %08X", refID);

    bool result = (bool) object->SetAngle(axis, value);

    if (result)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_ANGLE, object->GetNetworkID(), axis, value);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_SEQUENCED,
                                   CHANNEL_DATA,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(object);
}

void Game::GetParentCell(unsigned int refID, unsigned int cell)
{
    Object* object = (Object*) GameFactory::GetObject(refID);

    if (object == NULL)
        throw VaultException("Unknown object with reference %08X", refID);

    Player* self = (Player*) GameFactory::GetObject(PLAYER_REFERENCE);

    if (object != self)
    {
        if (self->GetGameCell() == object->GetNetworkCell() && object->GetGameCell() != object->GetNetworkCell())
        {
            Enable(object->GetNetworkID(), true);
            MoveTo(object->GetNetworkID(), self->GetNetworkID());
        }
    }

    bool result = (bool) object->SetGameCell(cell);

    if (result)
    {
        if (object != self)
        {
            if (object->GetNetworkCell() != self->GetGameCell() && !object->IsNearPoint(self->GetPos(Axis_X), self->GetPos(Axis_Y), self->GetPos(Axis_Z), 20000.0))
                Enable(object->GetNetworkID(), false);
            else
                Enable(object->GetNetworkID(), true);
        }
        else
        {
            pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_CELL, object->GetNetworkID(), cell);
            NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                       (unsigned char) HIGH_PRIORITY,
                                       (unsigned char) RELIABLE_SEQUENCED,
                                       CHANNEL_DATA,
                                       server));
            Network::Queue(response);
        }
    }

    GameFactory::LeaveReference(self);
    GameFactory::LeaveReference(object);
}

void Game::GetActorValue(unsigned int refID, bool base, unsigned char index, double value)
{
    Object* object = (Object*) GameFactory::GetObject(refID);

    if (object == NULL)
        throw VaultException("Unknown object with reference %08X", refID);

    Actor* actor = dynamic_cast<Actor*>(object);

    if (actor == NULL)
    {
        GameFactory::LeaveReference(object);
        throw VaultException("Object %08X is not an Actor", refID);
    }

    bool result;

    if (base)
        result = (bool) actor->SetActorBaseValue(index, value);
    else
        result = (bool) actor->SetActorValue(index, value);

    if (result)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_VALUE, actor->GetNetworkID(), base, index, value);
        NetworkResponse response = Network::CompleteResponse(Network::CreateResponse(packet,
                                   (unsigned char) HIGH_PRIORITY,
                                   (unsigned char) RELIABLE_ORDERED,
                                   CHANNEL_DATA,
                                   server));
        Network::Queue(response);
    }

    GameFactory::LeaveReference(actor);
}
