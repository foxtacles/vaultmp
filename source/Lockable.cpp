#include "Lockable.h"
#include "VaultException.h"

using namespace std;

unsigned int Lockable::key = 0x01;
unordered_map<unsigned int, Lockable*> Lockable::keymap;
unordered_map<unsigned int, weak_ptr<Lockable>> Lockable::sharemap;
CriticalSection Lockable::cs;

#ifdef VAULTMP_DEBUG
DebugInput<Lockable> Lockable::debug;
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
			throw VaultException("Lockable class ran out of keys").stacktrace();
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
		throw VaultException("Key %08X did not unlock anything", key).stacktrace();
	}

	cs.EndSession();

	return locked;
}

weak_ptr<Lockable> Lockable::Poll(unsigned int key, bool remove)
{
	weak_ptr<Lockable> shared;

	cs.StartSession();

	try
	{
		shared = sharemap.at(key);

		if (remove)
		{
			keymap.erase(key);
			sharemap.erase(key);
		}
	}
	catch (...)
	{
		cs.EndSession();
		throw VaultException("Key %08X did not share anything", key).stacktrace();
	}

	cs.EndSession();

	return shared;
}

bool Lockable::IsLocked() const
{
	return !locks.empty();
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
	keymap.emplace(next_key, this);

	cs.EndSession();

#ifdef VAULTMP_DEBUG
	debug.print(hex, this, " (", typeid(*this).name(), ") has been locked with key ", next_key);
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
		debug.print(hex, this, " (", typeid(*this).name(), ") has been unlocked with key ", key);
#endif

		return this;
	}

#ifdef VAULTMP_DEBUG
	debug.print(hex, this, " is still locked (key used: ", key, ")");
#endif

	return nullptr;
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

	sharemap.emplace(next_key, weak_ptr<Lockable>(share));
	keymap.emplace(next_key, share.get());

	cs.EndSession();

#ifdef VAULTMP_DEBUG
	debug.print(hex, share.get(), " (", typeid(*(share.get())).name(), ") has been shared with key ", next_key);
#endif

	return next_key;
}

