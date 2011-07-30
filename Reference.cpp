#include "Reference.h"

list<Reference*> Reference::references;

Reference::Reference(string ref, string base)
{
    this->ref = ref;
    this->base = base;
    references.push_back(this);
}

Reference::Reference()
{
    this->ref = "";
    this->base = "";
    references.push_back(this);
}

Reference::~Reference()
{
    references.remove(this);
}

list<Reference*> Reference::GetReferences()
{
    return references;
}

void Reference::SetReference(string ref)
{
    this->ref = ref;
}

void Reference::SetBase(string base)
{
    this->base = base;
}

string Reference::GetReference()
{
    return ref;
}

string Reference::GetBase()
{
    return base;
}
