#include "GameFactory.h"

CriticalSection GameFactory::cs;
multimap<unsigned char, Reference*> GameFactory::instances;

#ifdef VAULTMP_DEBUG
Debug* GameFactory::debug;
#endif

GameFactory::GameFactory()
{

}

#ifdef VAULTMP_DEBUG
void GameFactory::SetDebugHandler(Debug* debug)
{
    GameFactory::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to GameFactory class", true);
}
#endif

list<Reference*> GameFactory::GetObjectTypes(unsigned char type)
{
    cs.StartSession();

    list<Reference*> result;
    multimap<unsigned char, Reference*>::iterator it;
    pair<multimap<unsigned char, Reference*>::iterator, multimap<unsigned char, Reference*>::iterator> it2;
    it2 = instances.equal_range(type);

    for (it = it2.first; it != it2.second; ++it)
    {
        Reference* reference = (Reference*) it->second->StartSession();
        if (reference)
            result.push_back(reference);
    }

    cs.EndSession();

    return result;
}

Reference* GameFactory::GetObject(NetworkID id)
{
    Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);

    if (reference)
        reference = (Reference*) reference->StartSession();

    return reference;
}

Reference* GameFactory::GetObject(unsigned int refID)
{
    multimap<unsigned char, Reference*>::iterator it;

    cs.StartSession();

    for (it = instances.begin(); it != instances.end() && it->second->GetReference() != refID; ++it);
    Reference* reference = (it != instances.end() ? it->second : NULL);

    cs.EndSession();

    if (reference)
        reference = (Reference*) reference->StartSession();

    return reference;
}

void GameFactory::LeaveReference(Reference* reference)
{
    if (reference)
        reference->EndSession();
}

Reference* GameFactory::CreateInstance(unsigned char type, unsigned int refID, unsigned int baseID)
{
    Reference* reference;

    switch (type)
    {
    case ID_REFERENCE:
        throw VaultException("It is not possible to have a pure Reference instance");
    case ID_OBJECT:
        reference = new Object(refID, baseID);
        break;
    case ID_ITEM:
        reference = new Item(refID, baseID);
        break;
    case ID_CONTAINER:
        reference = new Container(refID, baseID);
        break;
    case ID_ACTOR:
        reference = new Actor(refID, baseID);
        break;
    case ID_PLAYER:
        reference = new Player(refID, baseID);
        break;
    default:
        throw VaultException("Unknown type identifier %X", type);
    }

    reference = (Reference*) reference->StartSession();

    cs.StartSession();
    instances.insert(pair<unsigned char, Reference*>(type, reference));
    cs.EndSession();

    return reference;
}

Reference* GameFactory::CreateInstance(unsigned char type, unsigned int baseID)
{
    return CreateInstance(type, 0x00, baseID);
}

void GameFactory::DestroyAllInstances()
{
    multimap<unsigned char, Reference*>::iterator it;

    cs.StartSession();

    for (it = instances.begin(); it != instances.end(); ++it)
    {
#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed", true, it->second->GetReference(), it->second->GetBase(), it->second->GetNetworkID(), typeid(*(it->second)).name());
#endif

        Reference* reference = (Reference*) it->second->StartSession();
        if (reference)
            delete reference;
    }

    instances.clear();

    cs.EndSession();
}

bool GameFactory::DestroyInstance(NetworkID id)
{
    Reference* reference = GetObject(id);

    if (!reference)
        return false;

    DestroyInstance(reference);

    return true;
}

NetworkID GameFactory::DestroyInstance(Reference* reference)
{
    NetworkID id = reference->GetNetworkID();

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed", true, reference->GetReference(), reference->GetBase(), reference->GetNetworkID(), typeid(*reference).name());
#endif

    cs.StartSession();

    delete reference;

    multimap<unsigned char, Reference*>::iterator it;

    for (it = instances.begin(); it != instances.end() && it->second != reference; ++it);

    if (it != instances.end())
        instances.erase(it);

    cs.EndSession();

    return id;
}
