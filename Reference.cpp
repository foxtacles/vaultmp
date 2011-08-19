#include "Reference.h"

list<Reference*> Reference::references;

Reference::Reference(unsigned int refID, unsigned int baseID)
{
    this->refID = refID;
    this->baseID = baseID;
    references.push_back(this);
}

Reference::Reference()
{
    this->refID = 0x00;
    this->baseID = 0x00;
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

void Reference::SetReference(unsigned int refID)
{
    this->refID = refID;
}

void Reference::SetBase(unsigned int baseID)
{
    this->baseID = baseID;
}

unsigned int Reference::GetReference()
{
    return refID;
}

unsigned int Reference::GetBase()
{
    return baseID;
}
