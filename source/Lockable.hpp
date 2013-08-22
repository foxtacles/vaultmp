#ifndef LOCKABLE_H
#define LOCKABLE_H

#include "vaultmp.hpp"
#include "CriticalSection.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <unordered_map>
#include <vector>
#include <memory>

/**
 * \brief An extension class which provides a basic lock / unlock mechanism
 *
 * The Value container class derives from this class
 */

class Lockable
{
	private:
		static unsigned int key;
		static std::unordered_map<unsigned int, Lockable*> keymap;
		static std::unordered_map<unsigned int, std::weak_ptr<Lockable>> sharemap;
		static std::unordered_map<const Lockable*, std::vector<unsigned int>> lockmap;
		static CriticalSection cs;

		static unsigned int NextKey();

#ifdef VAULTMP_DEBUG
		static DebugInput<Lockable> debug;
#endif

		Lockable& operator=(const Lockable&) = delete;

	protected:
		Lockable() = default;
		Lockable(Lockable&&) = default;
		Lockable& operator=(Lockable&&) = default;
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
		static std::weak_ptr<Lockable> Poll(unsigned int key, bool remove = true);

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
		static unsigned int Share(const std::shared_ptr<Lockable>& share);
		/**
		 * \brief Checks if this object is locked
		 *
		 * Usually used by classes which derive from Lockable to protect their members
		 */
		bool IsLocked() const;
};

#endif
