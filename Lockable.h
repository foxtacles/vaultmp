#ifndef LOCKABLE_H
#define LOCKABLE_H

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <climits>

#include "vaultmp.h"
#include "CriticalSection.h"
#include "VaultException.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#define ISFLAT(key) (key > 0x00)
#define ISDEEP(key) (key < 0x00)

using namespace std;

/**
 * \brief An extension class which provides a basic lock / unlock mechanism
 *
 * The Value container class derives from this class
 */

class Lockable
{

	private:
		static signed int flat_key;
		static signed int deep_key;
		static map<signed int, Lockable*> keymap;
		static CriticalSection cs;

		static signed int NextKey( bool flat );

		list<signed int> locks;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Lockable& operator=( const Lockable& );

	protected:

		Lockable();
		virtual ~Lockable();

	public:

		/**
		 * \brief Resets the class to its initial stays and releases all locks
		 */
		static void Reset();
		/**
		 * \brief Tries to unlock an object which has been locked with the given key
		 *
		 * Returns a pointer to the unlocked object if successful.
		 */
		static Lockable* BlindUnlock( signed int key );

		/**
		 * \brief Locks this object
		 *
		 * Returns a key on success.
		 *
		 * flat specifies the type of the lock.
		 * if flat = true, the key is necessary to remove this lock.
		 * if flat = false, the key can be used to remove all locks (regardless of their types) from this object
		 */
		signed int Lock( bool flat );
		/**
		 * \brief Unlocks this object
		 *
		 * Returns the object on success
		 *
		 * If the given key is of type "flat", the corresponding lock will be removed (the object may get unlocked)
		 * If the given key is of type "deep" (non-flat), the object will be unlocked and all locks will be removed from it
		 */
		Lockable* Unlock( signed int key );
		/**
		 * \brief Checks if this object is locked
		 *
		 * Usually used by classes which derive from Lockable to protect their members
		 */
		bool IsLocked() const;

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler( Debug* debug );
#endif

};

#endif
