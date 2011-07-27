#ifndef REFERENCE_H
#define REFERENCE_H

#include <string>
#include <list>

using namespace std;

class Reference
{

private:
    static list<Reference*> references;

    string ref;
    string base;

protected:
    Reference(string ref, string base);
    ~Reference();

    void SetReference(string ref);
    void SetBase(string base);

public:
    static list<Reference*> GetReferences();

    string GetReference();
    string GetBase();

};

#endif
