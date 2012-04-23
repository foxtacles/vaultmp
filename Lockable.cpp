#include "Lockable.h"

signed int Lockable::flat_key = 0x00 + 0x01;
signed int Lockable::deep_key = 0x00 - 0x01;
map<signed int, Lockable*> Lockable::keymap;
CriticalSection Lockable::cs;

#ifdef VAULTMP_DEBUG
Debug* Lockable::debug;
#endif

#ifdef VAULTMP_DEBUG
void Lockable::SetDebugHandler(Debug* debug)
{
	Lockable::debug = debug;

	if (debug != NULL)
		debug->Print("Attached debug handler to Lockable class", true);
}
#endif

Lockable::Lockable()
{

}

Lockable::~Lockable()
{

}

signed int Lockable::NextKey(bool flat)
{
	signed int next_key;

	cs.StartSession();

	switch (flat)
	{
		case true:
		{
			signed int temp_key = flat_key;

			while (keymap.find(temp_key) != keymap.end())
			{
				if (temp_key == INT_MAX)
					temp_key = 0x00 + 0x01;

				else
					temp_key++;

				if (temp_key == flat_key)
				{
					cs.EndSession();
					throw VaultException("Lockable class ran out of flat keys");
				}
			}

			next_key = temp_key;
			flat_key = (temp_key == INT_MAX ? (0x00 + 0x01) : (temp_key + 0x01));
			break;
		}

		case false:
		{
			signed int temp_key = deep_key;

			while (keymap.find(temp_key) != keymap.end())
			{
				if (temp_key == INT_MIN)
					temp_key = 0x00 - 0x01;

				else
					temp_key--;

				if (temp_key == deep_key)
				{
					cs.EndSession();
					throw VaultException("Lockable class ran out of deep keys");
				}
			}

			next_key = temp_key;
			deep_key = (temp_key == INT_MIN ? (0x00 - 0x01) : (temp_key - 0x01));
			break;
		}
	}

	cs.EndSession();

	return next_key;
}

void Lockable::Reset()
{
	keymap.clear();
	flat_key = 0x00 + 0x01;
	deep_key = 0x00 - 0x01;
}

Lockable* Lockable::BlindUnlock(signed int key)
{
	map<signed int, Lockable*>::iterator it;
	map<signed int, Lockable*> keymap_copy = keymap;

	for (it = keymap_copy.begin(); it != keymap_copy.end(); ++it)
	{
		Lockable* locked = it->second->Unlock(it->first);

		if (locked != NULL)
		{
			return locked;
		}
	}

#ifdef VAULTMP_DEBUG

	if (debug != NULL)
		debug->PrintFormat("Key %08X did not unlock anything", true, key);

#endif

	return NULL;
}

bool Lockable::IsLocked() const
{
	return locks.size();
}

signed int Lockable::Lock(bool flat)
{
	signed int next_key = NextKey(flat);

	if (next_key == 0x00)
		return next_key;

	cs.StartSession();
	locks.push_back(next_key);
	keymap.insert(pair<signed int, Lockable*>(next_key, this));
	cs.EndSession();

#ifdef VAULTMP_DEBUG

	if (debug != NULL)
		debug->PrintFormat("%08X (%s) has been locked with key %08X", true, this, typeid(*this).name(), next_key);

#endif

	return next_key;
}

Lockable* Lockable::Unlock(signed int key)
{
	if (!IsLocked())
		return this;

	if (find(locks.begin(), locks.end(), key) != locks.end())
	{
		cs.StartSession();

		if (ISDEEP(key))
		{
			list<signed int>::iterator it;

			for (it = locks.begin(); it != locks.end(); ++it)
				keymap.erase(*it);

			locks.clear();
		}

		else
		{
			keymap.erase(key);
			locks.remove(key);
		}

		cs.EndSession();

#ifdef VAULTMP_DEBUG

		if (debug != NULL)
			debug->PrintFormat("%08X (%s) has been unlocked with key %08X", true, this, typeid(*this).name(), key);

#endif

		if (!IsLocked())
			return this;
	}

#ifdef VAULTMP_DEBUG

	if (debug != NULL)
		debug->PrintFormat("%08X is still locked", true, this, key);

#endif

	return NULL;
}
