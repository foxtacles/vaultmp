#include "Lockable.h"

unsigned int Lockable::key = 0x01;
unordered_map<unsigned int, Lockable*> Lockable::keymap;
unordered_map<unsigned int, weak_ptr<Lockable>> Lockable::sharemap;
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
	unsigned int temp_key = key;

	while (keymap.find(temp_key) != keymap.end())
	{
		if (temp_key == UINT_MAX)
			temp_key = 0x01;
		else
			++temp_key;

		if (temp_key == key)
			throw VaultException("Lockable class ran out of keys");
	}

	next_key = temp_key;
	key = (temp_key == UINT_MAX ? (0x01) : (temp_key + 0x01));

	return next_key;
}

void Lockable::Reset()
{
	keymap.clear();
	key = 0x01;
}

Lockable* Lockable::Retrieve(unsigned int key)
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
		throw VaultException("Key %08X did not unlock anything", key);
	}

	cs.EndSession();

	return locked;
}

weak_ptr<Lockable> Lockable::Poll(unsigned int key)
{
	weak_ptr<Lockable> shared;

	cs.StartSession();

	try
	{
		shared = sharemap.at(key);
		keymap.erase(key);
		sharemap.erase(key);
	}
	catch (...)
	{
		cs.EndSession();
		throw VaultException("Key %08X did not share anything", key);
	}

	cs.EndSession();

	return shared;
}

bool Lockable::IsLocked() const
{
	return locks.size();
}

unsigned int Lockable::Lock()
{
	cs.StartSession();

	unsigned int next_key;

	try
	{
		next_key = NextKey();
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

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

unsigned int Lockable::Share(const shared_ptr<Lockable>& share)
{
	cs.StartSession();

	unsigned int next_key;

	try
	{
		next_key = NextKey();
	}
	catch (...)
	{
		cs.EndSession();
		throw;
	}

	sharemap.insert(pair<unsigned int, weak_ptr<Lockable>>(next_key, weak_ptr<Lockable>(share)));
	keymap.insert(pair<unsigned int, Lockable*>(next_key, share.get()));

	cs.EndSession();

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("%08X (%s) has been shared with key %08X", true, share.get(), typeid(*(share.get())).name(), next_key);

#endif

	return next_key;
}

