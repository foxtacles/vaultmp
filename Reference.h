#ifndef REFERENCE_H
#define REFERENCE_H

#include "Data.h"
#include "Utils.h"
#include "Value.h"
#include "Network.h"
#include "CriticalSection.h"
#include "VaultFunctor.h"
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

    Reference(const Reference&);
    Reference& operator=(const Reference&);

protected:
    Reference(unsigned int refID, unsigned int baseID);
    virtual ~Reference();

public:
    Lockable* SetReference(unsigned int refID);
    Lockable* SetBase(unsigned int baseID);

    unsigned int GetReference() const;
    unsigned int GetBase() const;

    const Parameter GetReferenceParam() const;
    const Parameter GetBaseParam() const;

};

#endif
