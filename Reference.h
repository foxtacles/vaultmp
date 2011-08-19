#ifndef REFERENCE_H
#define REFERENCE_H

#include "CriticalSection.h"

#include <list>

using namespace std;

class Reference : public CriticalSection
{

private:
    static list<Reference*> references;

    unsigned int refID;
    unsigned int baseID;

protected:
    Reference(unsigned int refID, unsigned int baseID);
    Reference();
    virtual ~Reference();

public:
    static list<Reference*> GetReferences();

    void SetReference(unsigned int refID);
    void SetBase(unsigned int baseID);

    unsigned int GetReference();
    unsigned int GetBase();

};

#endif
