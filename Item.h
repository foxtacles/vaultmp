#ifndef ITEM_H
#define ITEM_H

#define TYPECLASS
#include "GameFactory.h"

#include "vaultmp.h"
#include "Data.h"
#include "Value.h"
#include "Object.h"
#include "Container.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace Data;
using namespace std;

class Container;

class Item : public Object
{
friend class GameFactory;
friend class Container;

private:
#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Value<double> item_Condition;
    Value<Container*> item_Container;
    Value<bool> state_Worn;

    Item(const Item&);
    Item& operator=(const Item&);

protected:
    Item(unsigned int refID, unsigned int baseID);
    virtual ~Item();

public:
#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    double GetItemCondition() const;
    Container* GetItemContainer() const;
    bool GetItemWorn() const;

    bool SetItemCondition(double condition);
    bool SetItemContainer(Container* container);
    bool SetItemWorn(bool worn);

};

#endif
