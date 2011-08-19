#ifndef ACTOR_H
#define ACTOR_H

#include "Value.h"
#include "Inventory.h"
#include "VaultException.h"
#include "Utils.h"
#include "API.h"

#include <string>
#include <list>
#include <cmath>
#include <stdlib.h>

using namespace std;

class Actor : public Inventory
{

private:
    static map<unsigned int, Actor*> actorlist;
    static map<unsigned char, unsigned int> axis;
    static map<unsigned char, unsigned int> values;
    static map<unsigned char, unsigned int> anims;
    static bool initialized;

    static vector<string> GetRefs(bool enabled = true, bool enabled_disabled = false);
    unsigned int LookupIndex(map<unsigned char, unsigned int>& indices, unsigned char index);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Value<string> name;
    vector<Value<double> > actor_Pos;
    vector<Value<double> > actor_Angle;
    vector<Value<double> > actor_Values;
    vector<Value<double> > actor_BaseValues;
    vector<Value<bool> > actor_Animations;
    Value<unsigned int> cell_Game;
    Value<unsigned int> cell_Network;
    Value<bool> state_Enabled;
    Value<bool> state_Dead;

    void Startup(unsigned int refID, unsigned int baseID);

protected:
    Actor(unsigned int baseID);

public:
    Actor(unsigned int refID, unsigned int baseID);
    virtual ~Actor();

    static void Initialize();
    static void DestroyInstances();

    static map<RakNetGUID, Actor*> GetActorList();
    static Actor* GetActorFromRefID(unsigned int refID);
    static vector<string> GetAllRefs();
    static vector<string> GetEnabledRefs();
    static vector<string> GetDisabledRefs();

    static Parameter Param_EnabledActors;
    static Parameter Param_DisabledActors;
    static Parameter Param_AllActors;

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    string GetActorName();
    double GetActorPos(unsigned char axis);
    double GetActorAngle(unsigned char axis);
    unsigned int GetActorGameCell();
    unsigned int GetActorNetworkCell();
    double GetActorValue(unsigned char index);
    double GetActorBaseValue(unsigned char index);
    bool IsActorAnimationActive(unsigned char anim);
    bool GetActorEnabled();
    bool GetActorDead();

    Parameter GetActorRefParam();
    Parameter GetActorNameParam();

    bool SetActorName(string name);
    bool SetActorPos(unsigned char axis, double pos);
    bool SetActorAngle(unsigned char axis, double angle);
    bool SetActorGameCell(unsigned int cell);
    bool SetActorNetworkCell(unsigned int cell);
    bool SetActorValue(unsigned char index, double value);
    bool SetActorBaseValue(unsigned char index, double value);
    bool SetActorAnimation(unsigned char anim, bool state);
    bool SetActorEnabled(bool state);
    bool SetActorDead(bool state);

    bool IsActorNearPoint(double X, double Y, double Z, double R);
    bool IsCoordinateInRange(unsigned char axis, double value, double R);

};

#endif
