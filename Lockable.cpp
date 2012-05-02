#include "Lockable.h"

unsigned int Lockable::key = 0x01;
unordered_map<unsigned int, Lockable*> Lockable::keymap;
CriticalSection Lockable::cs;

#ifdef VAULTMP_DEBUG
Debug* Lockable::debug;
#endif

#ifdef VAULTMP_DEBUG
void Lockable::SetDebugHandler(Debug* debug)
{
	Lockable::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Lockable class", true);
}
#endif

unsigned int Lockable::NextKey()
{
	unsigned int next_key;

	cs.StartSession();

	unsigned int temp_key = key;

	while (keymap.find(temp_key) != keymap.end())
	{
		if (temp_key == UINT_MAX)
			temp_key = 0x01;
		else
			++temp_key;

		if (temp_key == key)
		{
			cs.EndSession();
			throw VaultException("Lockable class ran out of keys");
		}
	}

	next_key = temp_key;
	key = (temp_key == UINT_MAX ? (0x01) : (temp_key + 0x01));

	cs.EndSession();

	return next_key;
}

void Lockable::Reset()
{
	keymap.clear();
	key = 0x01;
}

Lockable* Lockable::BlindUnlock(unsigned int key)
{
	Lockable* locked;

	cs.StartSession();

	try
	{
		locked = keymap.at(key)->Unlock(key);
	}
	catch (...)
	{
		cs.EndSession();

#ifdef VAULTMP_DEBUG

		if (debug)
			debug->PrintFormat("Key %08X did not unlock anything", true, key);

#endif

		return NULL;
	}

	cs.EndSession();

	return locked;
}

bool Lockable::IsLocked() const
{
	return locks.size();
}

unsigned int Lockable::Lock()
{
	unsigned int next_key = NextKey();

	if (next_key == 0x00)
		return next_key;

	cs.StartSession();
	locks.insert(next_key);
	keymap.insert(pair<unsigned int, Lockable*>(next_key, this));
	cs.EndSession();

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("%08X (%s) has been locked with key %08X", true, this, typeid(*this).name(), next_key);

#endif

	return next_key;
}

Lockable* Lockable::Unlock(unsigned int key)
{
	if (!IsLocked())
		return this;

	if (locks.count(key))
	{
		locks.erase(key);

		cs.StartSession();
		keymap.erase(key);
		cs.EndSession();

#ifdef VAULTMP_DEBUG

		if (debug)
			debug->PrintFormat("%08X (%s) has been unlocked with key %08X", true, this, typeid(*this).name(), key);

#endif

		return this;
	}

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("%08X is still locked (key used: %08X)", true, this, key);

#endif

	return NULL;
}
