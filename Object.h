#ifndef OBJECT_H
#define OBJECT_H

#define TYPECLASS
#include "GameFactory.h"

#include "vaultmp.h"
#include "Data.h"
#include "API.h"
#include "Reference.h"
#include "Value.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace Data;
using namespace std;

class Object : public Reference
{
friend class GameFactory;

private:
#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Value<string> object_Name;
    map<unsigned char, Value<double> > object_Pos;
    map<unsigned char, Value<double> > object_Angle;
    Value<bool> state_Enabled;

protected:
    Object(unsigned int refID, unsigned int baseID);
    virtual ~Object();

public:
#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    static const Parameter Param_Axis;

    string GetName();
    double GetPos(unsigned char axis);
    double GetAngle(unsigned char axis);
    bool GetEnabled();

    bool SetName(string name);
    bool SetPos(unsigned char axis, double pos);
    bool SetAngle(unsigned char axis, double angle);
    bool SetEnabled(bool state);

};

#endif
