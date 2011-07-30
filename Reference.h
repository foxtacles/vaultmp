#ifndef REFERENCE_H
#define REFERENCE_H

#include "CriticalSection.h"

#include <string>
#include <list>

using namespace std;

class Reference : public CriticalSection
{

private:
    static list<Reference*> references;

    string ref;
    string base;

protected:
    Reference(string ref, string base);
    Reference();
    virtual ~Reference();

public:
    static list<Reference*> GetReferences();

    void SetReference(string ref);
    void SetBase(string base);

    string GetReference();
    string GetBase();

};

#endif
