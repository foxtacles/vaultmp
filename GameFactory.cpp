#include "GameFactory.h"

CriticalSection GameFactory::cs;
ReferenceList GameFactory::instances;

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

vector<Reference*> GameFactory::GetObjectTypes(unsigned char type)
{
    vector<Reference*> result;
    ReferenceList::iterator it;

    cs.StartSession();
    ReferenceList copy = instances;
    cs.EndSession();

    for (it = copy.begin(); it != copy.end() && (it->second & type); ++it)
    {
        Reference* reference = (Reference*) it->first->StartSession();
        if (reference)
            result.push_back(reference);
    }

    return result;
}

Reference* GameFactory::GetObject(unsigned char type, NetworkID id)
{
    return GetObject(ObjectNetwork(type, id));
}

Reference* GameFactory::GetObject(unsigned char type, unsigned int refID)
{
    return GetObject(ObjectReference(type, refID));
}

Reference* GameFactory::GetObject(ObjectNetwork object)
{
    cs.StartSession();
    Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(object.second);
    cs.EndSession();

    if (reference)
        reference = (Reference*) reference->StartSession();

    if (!reference)
        throw VaultException("Unknown object with NetworkID %lld", object.second);

    if ((GetType(reference) & object.first) == 0x00)
    {
        reference->EndSession();
        throw VaultException("Object is not of type %02X", object.first);
    }

    return reference;
}

Reference* GameFactory::GetObject(ObjectReference object)
{
    ReferenceList::iterator it;

    cs.StartSession();

    for (it = instances.begin(); it != instances.end() && it->first->GetReference() != object.second; ++it);
    Reference* reference = (it != instances.end() ? it->first : NULL);

    cs.EndSession();

    if (reference)
        reference = (Reference*) reference->StartSession();

    if (!reference)
        throw VaultException("Unknown object with reference %08X", object.second);

    if ((GetType(reference) & object.first) == 0x00)
    {
        reference->EndSession();
        throw VaultException("Object is not of type %02X", object.first);
    }

    return reference;
}

vector<Reference*> GameFactory::GetMultiple(const vector<ObjectNetwork>& objects)
{
    vector<Reference*> result;
    result.resize(objects.size());
    vector<ObjectNetwork>::const_iterator it;
    multimap<Reference*, unsigned int> sort;
    multimap<Reference*, unsigned int>::iterator it2;
    vector<Reference*>::iterator it3;
    int i = 0;

    cs.StartSession();

    for (it = objects.begin(); it != objects.end(); ++it, i++)
    {
        Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(it->second);

        if (!reference)
            throw VaultException("Unknown object with NetworkID %lld", it->second);

        if ((GetType(reference) & it->first) == 0x00)
            throw VaultException("Object is not of type %02X", it->first);

        sort.insert(pair<Reference*, unsigned int>(reference, i));
    }

    cs.EndSession();

    for (it2 = sort.begin(); it2 != sort.end(); ++it2)
    {
        Reference* reference = (Reference*) it2->first->StartSession();

        if (!reference)
        {
            for (it3 = result.begin(); it3 != result.end(); ++it3)
                (*it3)->EndSession();
            throw VaultException("Unknown object with NetworkID %lld", objects.at(it2->second).second);
        }

        result[it2->second] = reference;
    }

    return result;
}

vector<Reference*> GameFactory::GetMultiple(const vector<ObjectReference>& objects)
{
    vector<Reference*> result;
    result.resize(objects.size());
    vector<ObjectReference>::const_iterator it;
    multimap<Reference*, unsigned int> sort;
    multimap<Reference*, unsigned int>::iterator it2;
    vector<Reference*>::iterator it3;
    ReferenceList::iterator it4;
    int i = 0;

    cs.StartSession();

    for (it = objects.begin(); it != objects.end(); ++it, i++)
    {
        for (it4 = instances.begin(); it4 != instances.end() && it4->first->GetReference() != it->second; ++it4);
        Reference* reference = (it4 != instances.end() ? it4->first : NULL);

        if (!reference)
            throw VaultException("Unknown object with reference %08X", it->second);

        if ((GetType(reference) & it->first) == 0x00)
            throw VaultException("Object is not of type %02X", it->first);

        sort.insert(pair<Reference*, unsigned int>(reference, i));
    }

    cs.EndSession();

    for (it2 = sort.begin(); it2 != sort.end(); ++it2)
    {
        Reference* reference = (Reference*) it2->first->StartSession();

        if (!reference)
        {
            for (it3 = result.begin(); it3 != result.end(); ++it3)
                (*it3)->EndSession();
            throw VaultException("Unknown object with reference %08X", objects.at(it2->second).second);
        }

        result[it2->second] = reference;
    }

    return result;
}

NetworkID GameFactory::LookupNetworkID(unsigned int refID)
{
    ReferenceList::iterator it;

    cs.StartSession();
    for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);
    NetworkID id = (it != instances.end() ? it->first->GetNetworkID() : throw VaultException("Unknown object with reference %08X", refID));
    cs.EndSession();

    return id;
}

unsigned int GameFactory::LookupRefID(NetworkID id)
{
    cs.StartSession();
    Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);
    unsigned int refID = (reference != NULL ? reference->GetReference() : throw VaultException("Unknown object with NetworkID %lld", id));
    cs.EndSession();

    return refID;
}

unsigned char GameFactory::GetType(Reference* reference)
{
    ReferenceList::iterator it;

    cs.StartSession();

    unsigned char type;
    it = instances.find(reference);
    type = (it != instances.end() ? it->second : 0x00);

    cs.EndSession();

    return type;
}

void GameFactory::LeaveReference(Reference* reference)
{
    if (reference)
        reference->EndSession();
}

void GameFactory::LeaveReference(const vector<Reference*>& reference)
{
    vector<Reference*>::const_iterator it;
    for (it = reference.begin(); it != reference.end(); ++it)
        LeaveReference(*it);
}

NetworkID GameFactory::CreateInstance(unsigned char type, unsigned int refID, unsigned int baseID)
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

    NetworkID id = reference->GetNetworkID();

    cs.StartSession();
    instances.insert(pair<Reference*, unsigned char>(reference, type));
    cs.EndSession();

    return id;
}

NetworkID GameFactory::CreateInstance(unsigned char type, unsigned int baseID)
{
    return CreateInstance(type, 0x00, baseID);
}

void GameFactory::CreateKnownInstance(unsigned char type, NetworkID id, unsigned int refID, unsigned int baseID)
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

    reference->SetNetworkID(id);

    cs.StartSession();
    instances.insert(pair<Reference*, unsigned char>(reference, type));
    cs.EndSession();
}

void GameFactory::CreateKnownInstance(unsigned char type, NetworkID id, unsigned int baseID)
{
    return CreateKnownInstance(type, id, 0x00, baseID);
}

void GameFactory::DestroyAllInstances()
{
    ReferenceList::iterator it;

    cs.StartSession();

    for (it = instances.begin(); it != instances.end(); ++it)
    {
#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed", true, it->first->GetReference(), it->first->GetBase(), it->first->GetNetworkID(), typeid(*(it->first)).name());
#endif

        Reference* reference = (Reference*) it->first->StartSession();
        if (reference)
        {
            reference->Finalize();
            delete reference;
        }
    }

    instances.clear();

    cs.EndSession();

    // Cleanup classes

    Container::Cleanup();
    Object::param_Axis.first = vector<string>();
    Actor::param_ActorValues.first = vector<string>();
    Lockable::Reset();
}

bool GameFactory::DestroyInstance(NetworkID id)
{
    Reference* reference = (Reference*) GetObject(ALL_OBJECTS, id);

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

    ReferenceList::iterator it;
    it = instances.find(reference);

    if (it != instances.end())
        instances.erase(it);

    cs.EndSession();

    reference->Finalize();
    delete reference;

    return id;
}
