#ifndef ACTOR_H
#define ACTOR_H

#include "Inventory.h"

#include <string>
#include <list>
#include <math.h>

using namespace std;

class Actor : public Inventory
{

private:
    static map<string, Actor*> actorlist;
    static bool initialized;

    static vector<string> GetRefs(string cmd, bool enabled);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    string name;
    float pos[3];
    float angle;
    DWORD gcell;
    DWORD ncell;
    float health;
    float baseHealth;
    float cond[6];
    bool dead;
    bool alerted;
    int moving;

    bool enabled;
    bool nowrite[MAX_SKIP_FLAGS];

    void Startup(string ref, string base);

protected:
    Actor(string base);

public:
    Actor(string ref, string base);
    virtual ~Actor();

    static void Initialize();
    static void DestroyInstances();

    static map<RakNetGUID, Actor*> GetActorList();
    static Actor* GetActorFromRefID(string refID);
    static vector<string> GetAllRefs(string cmd);
    static vector<string> GetEnabledRefs(string cmd);

    static Parameter Param_EnabledActors;
    static Parameter Param_AllActors;

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    string GetActorName();
    float GetActorPos(int axis);
    float GetActorAngle();
    DWORD GetActorGameCell();
    DWORD GetActorNetworkCell();
    float GetActorHealth();
    float GetActorBaseHealth();
    float GetActorCondition(int cond);
    bool IsActorDead();
    bool IsActorAlerted();
    int GetActorMoving();
    bool GetActorEnabled();
    Parameter GetActorRefParam();
    Parameter GetActorNameParam();

    pActorUpdate GetActorUpdateStruct();
    pActorStateUpdate GetActorStateUpdateStruct();
    pActorCellUpdate GetActorCellUpdateStruct();
    pActorItemUpdate GetActorItemUpdateStruct(Item* item);
    bool UpdateActorUpdateStruct(pActorUpdate* data);
    bool UpdateActorStateUpdateStruct(pActorStateUpdate* data);
    bool UpdateActorCellUpdateStruct(pActorCellUpdate* data);
    bool UpdateActorItemUpdateStruct(list<pActorItemUpdate>* items, Inventory* inv);

    bool SetActorName(string name);
    bool SetActorPos(int axis, float pos);
    bool SetActorAngle(float angle);
    bool SetActorGameCell(DWORD cell);
    bool SetActorNetworkCell(DWORD cell);
    bool SetActorHealth(float health);
    bool SetActorBaseHealth(float baseHealth);
    bool SetActorCondition(int cond, float value);
    bool SetActorDead(bool dead);
    bool SetActorAlerted(bool alerted);
    bool SetActorMoving(int moving);
    bool SetActorEnabled(bool enabled);

    bool ToggleNoOverride(int skipflag, bool toggle);
    bool GetActorOverrideFlag(int skipflag);

    bool IsActorNearPoint(float X, float Y, float Z, float R);
    bool IsCoordinateInRange(int axis, float value, float R);

};

#endif
