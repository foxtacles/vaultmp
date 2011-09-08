#ifndef ACTOR_H
#define ACTOR_H

#define TYPECLASS
#include "GameFactory.h"

#include <string>
#include <list>
#include <cmath>
#include <typeinfo>
#include <cstdlib>

#include "Value.h"
#include "Container.h"
#include "VaultException.h"
#include "VaultFunctor.h"
#include "Utils.h"
#include "API.h"

const unsigned int FLAG_NOTSELF         = FLAG_DISABLED << 1;
const unsigned int FLAG_ALIVE           = FLAG_NOTSELF << 1;
const unsigned int FLAG_DEAD            = FLAG_ALIVE << 1;
const unsigned int FLAG_ISALERTED       = FLAG_DEAD << 1;
const unsigned int FLAG_NOTALERTED      = FLAG_ISALERTED << 1;

using namespace std;

class Actor : public Container
{
friend class GameFactory;

private:
    static Parameter param_ActorValues;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    map<unsigned char, Value<double> > actor_Values;
    map<unsigned char, Value<double> > actor_BaseValues;
    Value<unsigned char> anim_Running;
    Value<bool> state_Alerted;
    Value<bool> state_Dead;

    Actor(const Actor&);
    Actor& operator=(const Actor&);

protected:
    Actor(unsigned int refID, unsigned int baseID);
    virtual ~Actor();

public:
    static vector<Actor*> GetActorList();

    static const Parameter& Param_ActorValues();
    static const Parameter CreateFunctor(unsigned int flags);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    double GetActorValue(unsigned char index) const;
    double GetActorBaseValue(unsigned char index) const;
    unsigned char GetActorRunningAnimation() const;
    bool GetActorAlerted() const;
    bool GetActorDead() const;

    Lockable* SetActorValue(unsigned char index, double value);
    Lockable* SetActorBaseValue(unsigned char index, double value);
    Lockable* SetActorRunningAnimation(unsigned char index);
    Lockable* SetActorAlerted(bool state);
    Lockable* SetActorDead(bool state);

    bool IsJumping() const;

};

class ActorFunctor : public VaultFunctor
{
public:
    ActorFunctor(unsigned int flags) : VaultFunctor(flags) {};
    virtual ~ActorFunctor();

    virtual vector<string> operator()();
};

#endif
