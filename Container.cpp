#include "Container.h"
#include "Items.h"

bool Container::initialized = false;
unsigned char Container::game = 0x00;
ItemDatabase* Container::Items = NULL;
IndexLookup Container::Mods;

#ifdef VAULTMP_DEBUG
Debug* Container::debug;
#endif

#ifdef VAULTMP_DEBUG
void Container::SetDebugHandler(Debug* debug)
{
    Container::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Container class", true);
}
#endif

Container::Container(unsigned int refID, unsigned int baseID) : Object(refID, baseID)
{

}

Container::~Container()
{
    this->FlushContainer();
}

unsigned int Container::ResolveIndex(unsigned int baseID)
{
    unsigned char idx = (unsigned char) (((unsigned int) (baseID & 0xFF000000)) >> 24);
    IndexLookup::iterator it = Mods.find(idx);

    if (it != Mods.end())
        return (baseID & 0x00FFFFFF) | (((unsigned int) it->second) << 24);

    return baseID;
}

bool Container::Item_sort(NetworkID id, NetworkID id2)
{
    FactoryObject _item = GameFactory::GetObject(id);
    FactoryObject _item2 = GameFactory::GetObject(id2);
    Item* item = vaultcast<Item>(_item);
    Item* item2 = vaultcast<Item>(_item2);

    if (item->GetBase() > item2->GetBase())
        return false;
    else if (item->GetBase() == item2->GetBase())
    {
        if (item->GetItemEquipped() == item2->GetItemEquipped() && item->GetItemCondition() < item2->GetItemCondition())
            return false;
        else if (item2->GetItemEquipped())
            return false;
    }

    return true;
}

bool Container::Diff_sort(pair<unsigned int, Diff> diff, pair<unsigned int, Diff> diff2)
{
    if (diff.second.equipped > diff2.second.equipped)
        return false;

    return true;
}

StripCopy Container::Strip() const
{
    StripCopy result;
    result.first = this->Copy();

    FactoryObject _copy = GameFactory::GetObject(result.first);
    Container* copy = vaultcast<Container>(_copy);
    list<NetworkID> this_container = this->container;

    list<NetworkID>::iterator it, it2, it3, it4;

    for (it = this_container.begin(), it2 = copy->container.begin(); it != this_container.end() && it2 != copy->container.end(); ++it, ++it2)
    {
        pair<map<NetworkID, list<NetworkID> >::iterator, bool> it5;
        it5 = result.second.insert(pair<NetworkID, list<NetworkID> >(*it2, list<NetworkID>{*it}));

        FactoryObject _opt = GameFactory::GetObject(*it2);
        Item* opt = vaultcast<Item>(_opt);

        if (opt->GetItemEquipped())
            continue;

        for (++(it3 = it2), ++(it4 = it); it3 != copy->container.end() && it4 != this_container.end();)
        {
            FactoryObject _ins = GameFactory::GetObject(*it3);
            Item* ins = vaultcast<Item>(_ins);

            if (ins->GetBase() == opt->GetBase())
            {
                if (!ins->GetItemEquipped() && Utils::DoubleCompare(ins->GetItemCondition(), opt->GetItemCondition(), 0.001))
                {
                    opt->SetItemCount(opt->GetItemCount() + ins->GetItemCount());
                    GameFactory::DestroyInstance(_ins);
                    it5.first->second.push_back(*it4);
                    copy->container.erase(it3++);
                    this_container.erase(it4++);
                    continue;
                }
            }
            else
                break;

            ++it3;
            ++it4;
        }
    }

    return result;
}

void Container::Initialize(unsigned char game)
{
    if (!initialized)
    {
        Container::game = game;

        switch (game)
        {
        case FALLOUT3:
            Items = &Fallout3Items;
            break;
        case NEWVEGAS:
            Items = &FalloutNVItems;
            break;
        case OBLIVION:
            Items = &OblivionItems;
            break;
        default:
            throw VaultException("Bad game ID %08X", game);
        }

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Found %d items in the database", true, Items->size());
#endif

        initialized = true;
    }
}

void Container::Cleanup()
{
    if (initialized)
    {
        initialized = false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->Print("Performed cleanup on Container class", true);
#endif
    }
}

void Container::RegisterIndex(unsigned char real, unsigned char idx)
{

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Registered Fallout mod index (%02X => %02X)", true, real, idx);
#endif
}

void Container::AddItem(NetworkID id)
{
    FactoryObject reference = GameFactory::GetObject(id);

    if (vaultcast<Item>(reference))
    {
        container.push_back(id);
        container.sort(Item_sort);
    }
    else
        throw VaultException("Object with reference %08X is not an Item", (*reference)->GetReference());
}

void Container::RemoveItem(NetworkID id)
{
    list<NetworkID>::iterator it = find(container.begin(), container.end(), id);

    if (it == container.end())
        throw VaultException("Unknown Item with NetworkID %lld in Container with reference %08X", id, this->GetReference());

    container.erase(it);
}

ContainerDiff Container::Compare(NetworkID id) const
{
    FactoryObject reference = GameFactory::GetObject(id);
    Container* container = vaultcast<Container>(reference);

    if (!container)
        throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

    ContainerDiff diff;

    StripCopy _strip_self = this->Strip();
    map<NetworkID, list<NetworkID> >& _strip_assoc = _strip_self.second;
    FactoryObject _self = GameFactory::GetObject(_strip_self.first);
    FactoryObject _compare = GameFactory::GetObject(container->Strip().first);
    Container* self = vaultcast<Container>(_self);
    Container* compare = vaultcast<Container>(_compare);

    list<NetworkID>::iterator it, it2;

    for (it = compare->container.begin(), it2 = self->container.begin(); it != compare->container.end() && it2 != self->container.end();)
    {
        FactoryObject _iCompare = GameFactory::GetObject(*it);
        FactoryObject _iSelf = GameFactory::GetObject(*it2);
        Item* iCompare = vaultcast<Item>(_iCompare);
        Item* iSelf = vaultcast<Item>(_iSelf);

        unsigned int iCompare_base = iCompare->GetBase();
        unsigned int iSelf_base = iSelf->GetBase();

        if (iCompare_base == iSelf_base)
        {
            if (iCompare->GetItemEquipped() == iSelf->GetItemEquipped())
            {
                if (Utils::DoubleCompare(iCompare->GetItemCondition(), iSelf->GetItemCondition(), 0.001))
                {
                    if (iCompare->GetItemCount() == iSelf->GetItemCount()) // Item in self is existent in compare (match)
                    {
                        GameFactory::DestroyInstance(_iCompare);
                        GameFactory::DestroyInstance(_iSelf);
                        compare->container.erase(it++);
                        self->container.erase(it2++);
                        continue;
                    }
                }
            }
        }
        else if (iCompare_base < iSelf_base) // Item in compare is not existent in self (new / changed)
        {
            ++it;
            continue;
        }

        // Item in self is not existent in compare (delete / changed)
        ++it2;
    }

    for (it = self->container.begin(); it != self->container.end(); ++it)
    {
        list<NetworkID>& _delete = _strip_assoc.find(*it)->second;

        for (it2 = _delete.begin(); it2 != _delete.end(); ++it2)
            diff.first.push_back(*it2);
    }

    for (it = compare->container.begin(); it != compare->container.end(); compare->container.erase(it++))
        diff.second.push_back(*it);

    diff.first.sort(Item_sort);
    diff.second.sort(Item_sort);

    GameFactory::DestroyInstance(_self);
    GameFactory::DestroyInstance(_compare);
    return diff;
}

GameDiff Container::ApplyDiff(ContainerDiff& diff)
{
    GameDiff result;
    map<unsigned int, Diff> assoc_delete;

    list<NetworkID>::iterator it;
    map<unsigned int, Diff>::iterator it2;

    for (it = diff.first.begin(); it != diff.first.end(); ++it)
    {
        FactoryObject _iDelete = GameFactory::GetObject(*it);
        Item* iDelete = vaultcast<Item>(_iDelete);

        Diff* _diff = NULL;
        _diff = &assoc_delete.insert(pair<unsigned int, Diff>(iDelete->GetBase(), Diff())).first->second;

        _diff->count -= iDelete->GetItemCount();
        _diff->equipped -= iDelete->GetItemEquipped();

        this->RemoveItem(*it);
        GameFactory::DestroyInstance(_iDelete);
    }

    for (it = diff.second.begin(); it != diff.second.end(); ++it)
    {
        FactoryObject _iNew = GameFactory::GetObject(*it);
        Item* iNew = vaultcast<Item>(_iNew);

        Diff* _diff = NULL;
        it2 = assoc_delete.find(iNew->GetBase());

        if (it2 != assoc_delete.end())
        {
            _diff = &it2->second;

            if (iNew->GetItemEquipped() && _diff->equipped == -1)
            {
                Diff _result;
                _result.count = 0;
                _result.condition = iNew->GetItemCondition();
                _result.equipped = 0;
                result.push_back(pair<unsigned int, Diff>(iNew->GetBase(), _result));

                _diff->count += iNew->GetItemCount(); // always 1
                _diff->equipped = 0;
            }
            else
            {
                _diff->count += iNew->GetItemCount();
                _diff->condition = iNew->GetItemCondition();

                if (iNew->GetItemEquipped())
                    _diff->equipped = 1;
            }
        }
        else
        {
            Diff _result;
            _result.count = iNew->GetItemCount();
            _result.condition = iNew->GetItemCondition();
            _result.equipped = iNew->GetItemEquipped();
            result.push_back(pair<unsigned int, Diff>(iNew->GetBase(), _result));
        }

        this->AddItem(*it);
    }

    for (it2 = assoc_delete.begin(); it2 != assoc_delete.end(); ++it2)
    {
        if (it2->second.count == 0 && it2->second.equipped == 0)
            continue;

        result.push_back(pair<unsigned int, Diff>(*it2));
    }

    result.sort(Diff_sort);
    diff.first.clear();
    diff.second.clear();
    return result;
}

void FreeDiff(ContainerDiff& diff)
{
    list<NetworkID>::iterator it;

    for (it = diff.second.begin(); it != diff.second.end(); ++it)
    {
        FactoryObject _item = GameFactory::GetObject(*it);
        GameFactory::DestroyInstance(_item);
    }

    diff.first.clear();
    diff.second.clear();
}

NetworkID Container::Copy() const
{
    FactoryObject reference = GameFactory::GetObject(GameFactory::CreateInstance(ID_CONTAINER, 0x00000000, this->GetBase()));
    Container* container = vaultcast<Container>(reference);
    list<NetworkID>::const_iterator it;

    for (it = this->container.begin(); it != this->container.end(); ++it)
    {
        FactoryObject _reference = GameFactory::GetObject(*it);
        Item* item = vaultcast<Item>(_reference);
        container->container.push_back(item->Copy());
    }

    return container->GetNetworkID();
}

bool Container::IsEmpty() const
{
    return container.empty();
}

void Container::PrintContainer() const
{
#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        list<NetworkID>::const_iterator it;

        debug->PrintFormat("Content of container %08X (%s):", true, this->GetBase(), this->GetName().c_str());

        for (it = this->container.begin(); it != this->container.end(); ++it)
        {
            FactoryObject _reference = GameFactory::GetObject(*it);
            Item* item = vaultcast<Item>(_reference);
            debug->PrintFormat("%d of %s (%08X), condition %f, equipped state %d", true, item->GetItemCount(), item->ToString().c_str(), item->GetBase(), (float) item->GetItemCondition(), (int) item->GetItemEquipped());
        }
    }
#endif
}

void Container::FlushContainer()
{
    list<NetworkID>::iterator it;

    for (it = container.begin(); it != container.end(); ++it)
    {
        FactoryObject reference = GameFactory::GetObject(*it);
        GameFactory::DestroyInstance(reference);
    }

    container.clear();
}

const list<NetworkID>& Container::GetItemList() const
{
    return container;
}
