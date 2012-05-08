#ifndef LOCKABLE_H
#define LOCKABLE_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <climits>
#include <memory>

#include "vaultmp.h"
#include "CriticalSection.h"
#include "VaultException.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;

/**
 * \brief An extension class which provides a basic lock / unlock mechanism
 *
 * The Value container class derives from this class
 */

class Lockable
{

	private:
		static unsigned int key;
		static unordered_map<unsigned int, Lockable*> keymap;
		static unordered_map<unsigned int, weak_ptr<Lockable>> sharemap;
		static CriticalSection cs;

		static unsigned int NextKey();

		unordered_set<unsigned int> locks;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Lockable& operator=(const Lockable&);

	protected:

		Lockable() = default;
		Lockable(Lockable &&) = default;
		Lockable& operator=(Lockable &&) = default;
		virtual ~Lockable() {};

	public:

		/**
		 * \brief Resets the class to its initial state and releases all locks
		 */
		static void Reset();
		/**
		 * \brief Tries to unlock an object which has been locked with the given key
		 *
		 * Returns a pointer to the unlocked object if successful.
		 */
		static Lockable* Retrieve(unsigned int key);
		/**
		 * \brief Tries to obtain the weak pointer identified by key
		 *
		 * Returns a weak_ptr to the object if successful.
		 */
		static weak_ptr<Lockable> Poll(unsigned int key);

		/**
		 * \brief Locks this object
		 *
		 * Returns a key on success.
		 */
		unsigned int Lock();
		/**
		 * \brief Unlocks this object
		 *
		 * Returns the object on success
		 */
		Lockable* Unlock(unsigned int key);
		/**
		 * \brief Shares an object
		 *
		 * Returns a key on success.
		 */
		static unsigned int Share(const shared_ptr<Lockable>& share);
		/**
		 * \brief Checks if this object is locked
		 *
		 * Usually used by classes which derive from Lockable to protect their members
		 */
		bool IsLocked() const;

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
