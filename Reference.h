#ifndef REFERENCE_H
#define REFERENCE_H

#include "Data.h"
#include "Utils.h"
#include "Value.h"
#include "Network.h"
#include "CriticalSection.h"
#include "RakNet/NetworkIDObject.h"

using namespace std;
using namespace RakNet;
using namespace Data;

class Reference : public CriticalSection, public NetworkIDObject
{
friend class GameFactory;

private:
    Value<unsigned int> refID;
    Value<unsigned int> baseID;

protected:
    Reference(unsigned int refID, unsigned int baseID);
    virtual ~Reference();

public:
    void SetReference(unsigned int refID);
    void SetBase(unsigned int baseID);

    unsigned int GetReference();
    unsigned int GetBase();

    Parameter GetReferenceParam();
    Parameter GetBaseParam();

};

#endif
