#include "Game.h"

Player* Game::self = NULL;
RakNetGUID Game::server;
list<Player*> Game::refqueue;

#ifdef VAULTMP_DEBUG
Debug* Game::debug = NULL;
#endif

#ifdef VAULTMP_DEBUG
void Game::SetDebugHandler(Debug* debug)
{
    Game::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Game class", true);
}
#endif

NetworkResponse Game::Authenticate()
{
    pDefault* packet = PacketFactory::CreatePacket(ID_GAME_AUTH, self->GetActorName().c_str(), self->GetPlayerPassword().c_str());
    return Network::CompleteResponse(Network::CreateResponse(packet,
                                                             (unsigned char) HIGH_PRIORITY,
                                                             (unsigned char) RELIABLE_ORDERED,
                                                             CHANNEL_GAME,
                                                             server));
}

void Game::InitializeCommands()
{
    refqueue.clear();

    Interface::DefineCommand("GetPos", "%0.GetPos %1");
    Interface::DefineCommand("SetPos", "%0.SetPos %1 %2");
    Interface::DefineCommand("GetAngle", "%0.GetAngle %1");
    Interface::DefineCommand("SetAngle", "%0.SetAngle %1 %2");
    Interface::DefineCommand("GetParentCell", "%0.GetParentCell");
    Interface::DefineCommand("GetBaseActorValue", "%0.GetBaseActorValue %1");
    Interface::DefineCommand("SetActorValue", "%0.SetActorValue %1 %2");
    Interface::DefineCommand("ForceActorValue", "%0.ForceActorValue %1 %2");
    Interface::DefineCommand("GetActorValue", "%0.GetActorValue %1");
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
    Interface::DefineCommand("MoveAllItems", "%0.RemoveAllItems %1 %2", "RemoveAllItems");
    Interface::DefineCommand("RemoveAllItems", "%0.RemoveAllItems");
    Interface::DefineCommand("KillActor", "%0.Kill %1 %2 %3", "Kill");
    Interface::DefineCommand("Kill", "%0.Kill");
    Interface::DefineCommand("PlaceAtMe", "%0.PlaceAtMe %1 %2 %3 %4");
    Interface::DefineCommand("MarkForDelete", "%0.MarkForDelete");
    Interface::DefineCommand("Load", "Load %0");

    Interface::StartSession();

    ParamList param_GetPos;
    param_GetPos.push_back(Player::Param_EnabledPlayers);
    param_GetPos.push_back(Actor::Param_Axis);
    ParamContainer GetPos = ParamContainer(param_GetPos, &Data::AlwaysTrue);
    Interface::DefineNative("GetPos", GetPos);
    Interface::ExecuteCommandLoop("GetPos");

    ParamList param_GetAngle;
    param_GetAngle.push_back(self->GetActorRefParam());
    param_GetAngle.push_back(Actor::Param_Axis);
    ParamContainer GetAngle = ParamContainer(param_GetAngle, &Data::AlwaysTrue);
    Interface::DefineNative("GetAngle", GetAngle);
    Interface::ExecuteCommandLoop("GetAngle");

    ParamList param_GetParentCell;
    param_GetParentCell.push_back(Player::Param_AllPlayers);
    ParamContainer GetParentCell = ParamContainer(param_GetParentCell, &Data::AlwaysTrue);
    Interface::DefineNative("GetParentCell", GetParentCell);
    Interface::ExecuteCommandLoop("GetParentCell");

    ParamList param_GetDead;
    param_GetDead.push_back(Player::Param_EnabledPlayers);
    ParamContainer GetDead = ParamContainer(param_GetDead, &Data::AlwaysTrue);
    Interface::DefineNative("GetDead", GetDead);
    Interface::ExecuteCommandLoop("GetDead");

    ParamList param_IsMoving;
    param_IsMoving.push_back(self->GetActorRefParam());
    ParamContainer IsMoving = ParamContainer(param_IsMoving, &Data::AlwaysTrue);
    Interface::DefineNative("IsMoving", IsMoving);
    Interface::ExecuteCommandLoop("IsMoving");

    ParamList param_GetActorValue;
    param_GetActorValue.push_back(self->GetActorRefParam());
    param_GetActorValue.push_back(Actor::Param_ActorValues);
    ParamContainer GetActorValue = ParamContainer(param_GetActorValue, &Data::AlwaysTrue);
    Interface::DefineNative("GetActorValue", GetActorValue);
    Interface::ExecuteCommandLoop("GetActorValue", 10);

    ParamList param_GetBaseActorValue;
    param_GetBaseActorValue.push_back(self->GetActorRefParam());
    param_GetBaseActorValue.push_back(Actor::Param_ActorValues);
    ParamContainer GetBaseActorValue = ParamContainer(param_GetBaseActorValue, &Data::AlwaysTrue);
    Interface::DefineNative("GetBaseActorValue", GetBaseActorValue);
    Interface::ExecuteCommandLoop("GetBaseActorValue", 100);

    Interface::EndSession();
}
