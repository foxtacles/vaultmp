#ifndef ACTOR_H
#define ACTOR_H

#define TYPECLASS
#include "GameFactory.h"

#include "Value.h"
#include "Container.h"
#include "VaultException.h"
#include "Utils.h"
#include "API.h"

#include <string>
#include <list>
#include <cmath>
#include <typeinfo>
#include <stdlib.h>

using namespace std;

class Actor : public Container
{
friend class GameFactory;

private:
    static vector<string> GetRefs(bool enabled = true, bool enabled_disabled = false);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    map<unsigned char, Value<double> > actor_Values;
    map<unsigned char, Value<double> > actor_BaseValues;
    map<unsigned char, Value<bool> > actor_Animations;
    Value<unsigned int> cell_Game;
    Value<unsigned int> cell_Network;
    Value<bool> state_Dead;

    void Startup();

protected:
    Actor(unsigned int refID, unsigned int baseID);
    virtual ~Actor();

public:
    static list<Actor*> GetActorList();
    static vector<string> GetAllRefs();
    static vector<string> GetEnabledRefs();
    static vector<string> GetDisabledRefs();

    static const Parameter Param_EnabledActors;
    static const Parameter Param_DisabledActors;
    static const Parameter Param_AllActors;
    static const Parameter Param_ActorValues;

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    unsigned int GetActorGameCell();
    unsigned int GetActorNetworkCell();
    double GetActorValue(unsigned char index);
    double GetActorBaseValue(unsigned char index);
    bool IsActorAnimationActive(unsigned char anim);
    bool GetActorDead();

    bool SetActorGameCell(unsigned int cell);
    bool SetActorNetworkCell(unsigned int cell);
    bool SetActorValue(unsigned char index, double value);
    bool SetActorBaseValue(unsigned char index, double value);
    bool SetActorAnimation(unsigned char anim, bool state);
    bool SetActorDead(bool state);

    bool IsActorNearPoint(double X, double Y, double Z, double R);
    bool IsCoordinateInRange(unsigned char axis, double value, double R);

};

#endif
