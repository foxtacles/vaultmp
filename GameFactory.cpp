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

vector<FactoryObject> GameFactory::GetObjectTypes(unsigned char type)
{
    vector<FactoryObject> result;
    ReferenceList::iterator it;

    cs.StartSession();
    ReferenceList copy = instances;
    cs.EndSession();

    for (it = copy.begin(); it != copy.end() && (it->second & type); ++it)
        result.push_back(FactoryObject(it->first));

    return result;
}

FactoryObject GameFactory::GetObject(NetworkID id)
{
    cs.StartSession();
    Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(id);
    cs.EndSession();

    if (!reference)
        throw VaultException("Unknown object with NetworkID %lld", id);

    return FactoryObject(reference);
}

FactoryObject GameFactory::GetObject(unsigned int refID)
{
    ReferenceList::iterator it;

    cs.StartSession();
    for (it = instances.begin(); it != instances.end() && it->first->GetReference() != refID; ++it);
    Reference* reference = (it != instances.end() ? it->first : NULL);
    cs.EndSession();

    if (!reference)
        throw VaultException("Unknown object with reference %08X", refID);

    return FactoryObject(reference);
}

vector<FactoryObject> GameFactory::GetMultiple(const vector<NetworkID>& objects)
{
    vector<FactoryObject> result;
    result.resize(objects.size(), FactoryObject());
    vector<NetworkID>::const_iterator it;
    multimap<Reference*, unsigned int> sort;
    multimap<Reference*, unsigned int>::iterator it2;
    int i = 0;

    cs.StartSession();

    for (it = objects.begin(); it != objects.end(); ++it, i++)
    {
        Reference* reference = Network::Manager()->GET_OBJECT_FROM_ID<Reference*>(*it);

        if (!reference)
            throw VaultException("Unknown object with NetworkID %lld", *it);

        sort.insert(pair<Reference*, unsigned int>(reference, i));
    }

    cs.EndSession();

    for (it2 = sort.begin(); it2 != sort.end(); ++it2)
        result[it2->second] = FactoryObject(it2->first);

    return result;
}

vector<FactoryObject> GameFactory::GetMultiple(const vector<unsigned int>& objects)
{
    vector<FactoryObject> result;
    result.resize(objects.size(), FactoryObject());
    vector<unsigned int>::const_iterator it;
    multimap<Reference*, unsigned int> sort;
    multimap<Reference*, unsigned int>::iterator it2;
    ReferenceList::iterator it3;
    int i = 0;

    cs.StartSession();

    for (it = objects.begin(); it != objects.end(); ++it, i++)
    {
        for (it3 = instances.begin(); it3 != instances.end() && it3->first->GetReference() != *it; ++it3);
        Reference* reference = (it3 != instances.end() ? it3->first : NULL);

        if (!reference)
            throw VaultException("Unknown object with reference %08X", *it);

        sort.insert(pair<Reference*, unsigned int>(reference, i));
    }

    cs.EndSession();

    for (it2 = sort.begin(); it2 != sort.end(); ++it2)
        result[it2->second] = FactoryObject(it2->first);

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

void GameFactory::LeaveReference(FactoryObject& reference)
{
    Reference* _reference = reference.reference;

    if (!_reference)
        throw VaultException("GameFactory::LeaveReference Reference is NULL");

    _reference->EndSession();
    reference.reference = NULL;
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
        if (it->second == ID_CONTAINER)
            vaultcast<Container>(it->first)->container.clear();

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
    FactoryObject reference = GetObject(id);
    DestroyInstance(reference);
    return true;
}

NetworkID GameFactory::DestroyInstance(FactoryObject& reference)
{
    Reference* _reference = reference.reference;

    if (!_reference)
        throw VaultException("GameFactory::DestroyInstance Reference is NULL");

    NetworkID id = _reference->GetNetworkID();

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Reference %08X with base %08X and NetworkID %lld (type: %s) to be destructed", true, _reference->GetReference(), _reference->GetBase(), _reference->GetNetworkID(), typeid(*_reference).name());
#endif

    cs.StartSession();

    ReferenceList::iterator it;
    it = instances.find(_reference);

    if (it != instances.end())
        instances.erase(it);

    cs.EndSession();

    _reference->Finalize();
    delete _reference;
    reference.reference = NULL;

    return id;
}
