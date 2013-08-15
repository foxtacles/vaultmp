#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

#include "vaultmp.h"
#include "Reference.h"
#include "Expected.h"

#ifdef VAULTSERVER
#include "vaultserver/Database.h"
#include "vaultserver/Record.h"
#include "vaultserver/Reference.h"
#include "vaultserver/Exterior.h"
#include "vaultserver/Weapon.h"
#include "vaultserver/Race.h"
#include "vaultserver/NPC.h"
#include "vaultserver/BaseContainer.h"
#include "vaultserver/Item.h"
#include "vaultserver/Terminal.h"
#include "vaultserver/Interior.h"
#endif

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include <map>
#include <memory>
#include <unordered_set>

/**
  * \brief Holds an instance pointer
  */
template<typename T> class FactoryWrapper;
template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept;
template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast(const Expected<FactoryWrapper<U>>& object) noexcept;
template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>& object) noexcept;
template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast_swap(Expected<FactoryWrapper<U>>& object) noexcept;
template<typename T> struct rTypes;

class Object;

/**
 * \brief Create, use and destroy game object instances via the GameFactory
 */
class GameFactory
{
	private:
		typedef std::map<std::shared_ptr<Reference>, unsigned int> ReferenceList;
		typedef std::unordered_map<RakNet::NetworkID, ReferenceList::iterator> ReferenceIndex;
		typedef std::unordered_map<unsigned int, unsigned int> ReferenceCount;
		typedef std::unordered_set<RakNet::NetworkID> ReferenceDeleted;

		GameFactory() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<GameFactory> debug;
#endif

		static CriticalSection cs;
		static ReferenceList instances;
		static ReferenceIndex index;
		static ReferenceCount typecount;
		static ReferenceDeleted delrefs;
		static bool changed;

#ifdef VAULTSERVER
		static Database<DB::Record> dbRecords;
		static Database<DB::Reference> dbReferences;
		static Database<DB::Exterior> dbExteriors;
		static Database<DB::Weapon> dbWeapons;
		static Database<DB::Race> dbRaces;
		static Database<DB::NPC> dbNpcs;
		static Database<DB::BaseContainer> dbContainers;
		static Database<DB::Item> dbItems;
		static Database<DB::Terminal> dbTerminals;
		static Database<DB::Interior> dbInteriors;
#endif

		inline static ReferenceList::iterator GetShared(const RakNet::NetworkID& id) { return index.count(id) ? index[id] : instances.end(); }

	public:
		enum class FailPolicy
		{
			Return,
			Bool,
			Exception,
			Default = Exception
		};

		enum class LaunchPolicy
		{
			Blocking,
			Async,
			Default = Blocking
		};

		enum class ObjectPolicy
		{
			Validated,
			Expected,
			Default = Validated
		};

	private:
		template<ObjectPolicy OP, typename T> struct ObjectPolicyType;
		template<typename T> struct ObjectPolicyType<ObjectPolicy::Validated, T> { typedef FactoryWrapper<T> type; };
		template<typename T> struct ObjectPolicyType<ObjectPolicy::Expected, T> { typedef Expected<FactoryWrapper<T>> type; };

		template<ObjectPolicy OP, typename T, typename F, bool M> struct ObjectPolicyReturn;
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Validated, T, F, false> { typedef typename std::result_of<F(FactoryWrapper<T>&)>::type type; };
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Expected, T, F, false> { typedef typename std::result_of<F(Expected<FactoryWrapper<T>>&)>::type type; };
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Validated, T, F, true> { typedef typename std::result_of<F(std::vector<FactoryWrapper<T>>&)>::type type; };
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Expected, T, F, true> { typedef typename std::result_of<F(std::vector<Expected<FactoryWrapper<T>>>&)>::type type; };

		template<FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename T, typename F, bool M> struct OperateReturn;
		template<ObjectPolicy OP, typename T, typename F, bool M> struct OperateReturn<FailPolicy::Bool, OP, LaunchPolicy::Blocking, T, F, M> { typedef bool type; };
		template<typename T, typename F, bool M> struct OperateReturn<FailPolicy::Return, ObjectPolicy::Validated, LaunchPolicy::Blocking, T, F, M> { typedef typename ObjectPolicyReturn<ObjectPolicy::Validated, T, F, M>::type type; };
		template<typename T, typename F, bool M> struct OperateReturn<FailPolicy::Return, ObjectPolicy::Expected, LaunchPolicy::Blocking, T, F, M> { typedef typename ObjectPolicyReturn<ObjectPolicy::Expected, T, F, M>::type type; };
		template<typename T, typename F, bool M> struct OperateReturn<FailPolicy::Exception, ObjectPolicy::Validated, LaunchPolicy::Blocking, T, F, M> { typedef typename ObjectPolicyReturn<ObjectPolicy::Validated, T, F, M>::type type; };
		template<typename T, typename F, bool M> struct OperateReturn<FailPolicy::Exception, ObjectPolicy::Expected, LaunchPolicy::Blocking, T, F, M> { typedef typename ObjectPolicyReturn<ObjectPolicy::Expected, T, F, M>::type type; };

		template<typename P, FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename T, typename F, bool M> struct OperateReturnProxy { typedef typename OperateReturn<FP, OP, LP, T, F, M>::type type; };

		template<typename T, FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename I, typename F, bool M> struct OperateFunctions;

		template<typename T, FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename I, typename F>
		struct OperateFunctions<T, FP, OP, LP, I, F, false> {
			static typename OperateReturn<FP, OP, LP, T, F, false>::type Operate(I&& id, F function) noexcept(FP != FailPolicy::Exception);
		};

		template<typename T, FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename I, typename F>
		struct OperateFunctions<T, FP, OP, LP, I, F, true> {
			static typename OperateReturn<FP, OP, LP, T, F, true>::type Operate(std::vector<typename I::value_type>&& id, F function) noexcept(FP != FailPolicy::Exception);
		};

		template<ObjectPolicy OP, typename T>
		struct ObjectPolicyHelper {
			inline static typename ObjectPolicyType<OP, T>::type& Unwrap(Expected<FactoryWrapper<T>>& reference) noexcept(OP == ObjectPolicy::Expected);
			inline static typename ObjectPolicyType<OP, T>::type& Unwrap(std::vector<Expected<FactoryWrapper<T>>>& reference) noexcept(OP == ObjectPolicy::Expected);
		};

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::Validated, T> {
			inline static typename ObjectPolicyType<ObjectPolicy::Validated, T>::type& Unwrap(Expected<FactoryWrapper<T>>& reference) { return reference.get(); }
			inline static typename std::vector<typename ObjectPolicyType<ObjectPolicy::Validated, T>::type> Unwrap(std::vector<Expected<FactoryWrapper<T>>>& reference) {
				// a solution with std::reference_wrapper would also be possible. not sure which would perform better
				std::vector<FactoryWrapper<T>> result;
				result.reserve(reference.size());

				for (auto& reference_ : reference) {
					auto& reference__ = reference_.get();
					result.emplace_back(reference__);
					GameFactory::LeaveReference(reference__);
				}

				return result;
			}
		};

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::Expected, T> {
			inline static typename ObjectPolicyType<ObjectPolicy::Expected, T>::type& Unwrap(Expected<FactoryWrapper<T>>& reference) noexcept { return reference; }
			inline static typename std::vector<typename ObjectPolicyType<ObjectPolicy::Expected, T>::type>& Unwrap(std::vector<Expected<FactoryWrapper<T>>>& reference) noexcept { return reference; }
		};

		template<typename I>
		struct InputPolicyHelper {
			static constexpr bool M = !std::is_trivial<typename std::remove_reference<I>::type>::value && !std::is_base_of<FactoryWrapper<Reference>, typename std::remove_reference<I>::type>::value;
		};

	public:
		static void Initialize();

		/**
		 * \brief Obtains a lock on a Reference
		 *
		 * The Reference is identified by a NetworkID
		 */
		template<typename T = Object, typename I = RakNet::NetworkID>
		static Expected<FactoryWrapper<T>> GetObject(I id) noexcept;
		/**
		 * \brief This is an alias to vaultcast_swap
		 */
		template<typename T, typename U> inline static Expected<FactoryWrapper<T>> GetObject(FactoryWrapper<U>& reference) noexcept { return vaultcast_swap<T>(reference); }
		template<typename T, typename U> inline static Expected<FactoryWrapper<T>> GetObject(Expected<FactoryWrapper<U>>& reference) noexcept { return vaultcast_swap<T>(reference); }
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of NetworkID. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		template<typename T = Object, typename I = RakNet::NetworkID>
		static std::vector<Expected<FactoryWrapper<T>>> GetMultiple(const std::vector<I>& ids) noexcept;
		/**
		 * \brief Executes a function on one or multiple References
		 *
		 * The References are identified by an arbitrary type. This can be a NetworkID, reference ID or FactoryWrapper
		 */
		template<typename T = Object, FailPolicy FP = FailPolicy::Default, ObjectPolicy OP = ObjectPolicy::Default, LaunchPolicy LP = LaunchPolicy::Default, typename I, typename F>
		static typename OperateReturn<FP, OP, LP, T, F, InputPolicyHelper<I>::M>::type Operate(I&& id, F function) { return OperateFunctions<T, FP, OP, LP, I, F, InputPolicyHelper<I>::M>::Operate(std::forward<I>(id), function); }
		/**
		 * \brief Checks if an ID has been deleted
		 */
		static bool IsDeleted(RakNet::NetworkID id) noexcept;
		/**
		 * \brief Returns the type of the given NetworkID
		 */
		static unsigned int GetType(RakNet::NetworkID id) noexcept;
		/**
		 * \brief Returns the type of the given reference
		 */
		static unsigned int GetType(const Reference* reference) noexcept;
		/**
		 * \brief Obtains a lock on all References of a given type
		 */
		template<typename T = Object>
		static std::vector<FactoryWrapper<T>> GetObjectTypes(unsigned int type) noexcept;
		/**
		 * \brief Returns the NetworkID's of all References of a given type
		 */
		static std::vector<RakNet::NetworkID> GetIDObjectTypes(unsigned int type) noexcept;
		/**
		 * \brief Counts the amount of References of a given type
		 */
		static unsigned int GetObjectCount(unsigned int type) noexcept;
		/**
		 * \brief Invalidates a Reference held by a FactoryWrapper
		 */
		template<typename T>
		static void LeaveReference(FactoryWrapper<T>& reference);
		/**
		 * \brief Creates a new instance of a given type
		 */
		template<typename T = Object>
		static RakNet::NetworkID CreateInstance();
		/**
		 * \brief Creates a known instance from a network packet
		 */
		template<typename T = Object>
		static RakNet::NetworkID CreateKnownInstance(const pDefault* packet);

		/**
		 * \brief Destroys all instances and cleans up type classes
		 */
		static void DestroyAllInstances() noexcept;
		/**
		 * \brief Destroys an instance
		 */
		static bool DestroyInstance(RakNet::NetworkID id);
		/**
		 * \brief Destroys an instance which has previously been locked
		 *
		 * You must make sure the lock count of the given Reference equals to one
		 */
		template<typename T>
		static RakNet::NetworkID DestroyInstance(FactoryWrapper<T>& reference);
};

template<typename T>
Expected<FactoryWrapper<T>> GameFactory::GetObject<T, RakNet::NetworkID>(RakNet::NetworkID id) noexcept
{
	std::pair<ReferenceList::key_type, ReferenceList::mapped_type> reference;

	cs.StartSession();

	auto it = GetShared(id);

	if (it != instances.end())
		reference = *it;

	cs.EndSession();

	if (!reference.first)
		return VaultException("Unknown object with NetworkID %llu", id);

	return FactoryWrapper<T>(reference.first.get(), reference.second);
}

template<typename T, typename I>
inline Expected<FactoryWrapper<T>> GameFactory::GetObject<T, I>(I id) noexcept
{
	return GetObject<T>(T::PickBy<I>(id));
}

template<typename T>
std::vector<Expected<FactoryWrapper<T>>> GameFactory::GetMultiple<T, RakNet::NetworkID>(const std::vector<RakNet::NetworkID>& ids) noexcept
{
	std::vector<Expected<FactoryWrapper<T>>> result(ids.size());
	std::multimap<ReferenceList::value_type, unsigned int> sort;

	cs.StartSession();

	unsigned int i = 0;

	for (const auto& id : ids)
	{
		auto it = GetShared(id);

		if (it == instances.end())
			result[i] = VaultException("Unknown object with NetworkID %llu", id);
		else
			// emplace
			sort.insert(make_pair(*it, i));

		++i;
	}

	cs.EndSession();

	for (const auto& reference : sort)
		result[reference.second] = FactoryWrapper<T>(reference.first.first.get(), reference.first.second);

	return result;
}

template<typename T, typename I>
inline std::vector<Expected<FactoryWrapper<T>>> GameFactory::GetMultiple<T, I>(const std::vector<ID>& ids) noexcept
{
	return GetMultiple<T>(T::PickBy<I>(ids));
}

using FailPolicy = GameFactory::FailPolicy;
using LaunchPolicy = GameFactory::LaunchPolicy;
using ObjectPolicy = GameFactory::ObjectPolicy;

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Return, OP, LaunchPolicy::Blocking, I, F, false> {
	static typename OperateReturn<FailPolicy::Return, OP, LaunchPolicy::Blocking, T, F, false>::type Operate(I&& id, F function) noexcept
	{
		auto reference = GameFactory::GetObject<T>(std::forward<I>(id));

		try
		{
			return function(ObjectPolicyHelper<OP, T>::Unwrap(reference));
		}
		catch (...) { return typename ObjectPolicyReturn<OP, T, F, false>::type(); }
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Bool, OP, LaunchPolicy::Blocking, I, F, false> {
	static typename OperateReturn<FailPolicy::Bool, OP, LaunchPolicy::Blocking, T, F, false>::type Operate(I&& id, F function) noexcept
	{
		static_assert(std::is_same<typename ObjectPolicyReturn<OP, T, F, false>::type, void>::value, "Function return value disregarded");

		auto reference = GameFactory::GetObject<T>(std::forward<I>(id));

		try
		{
			function(ObjectPolicyHelper<OP, T>::Unwrap(reference));
		}
		catch (...) { return false; }

		return true;
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Exception, OP, LaunchPolicy::Blocking, I, F, false> {
	static typename OperateReturn<FailPolicy::Exception, OP, LaunchPolicy::Blocking, T, F, false>::type Operate(I&& id, F function)
	{
		auto reference = GameFactory::GetObject<T>(std::forward<I>(id));
		return function(ObjectPolicyHelper<OP, T>::Unwrap(reference));
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Return, OP, LaunchPolicy::Blocking, I, F, true> {
	static typename OperateReturn<FailPolicy::Return, OP, LaunchPolicy::Blocking, T, F, true>::type Operate(std::vector<typename I::value_type>&& id, F function) noexcept
	{
		auto reference = GameFactory::GetMultiple<T>(std::forward<I>(id));

		try
		{
			auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(reference); // depending on OP, returns either a reference or value
			return function(param);
		}
		catch (...) { return typename ObjectPolicyReturn<OP, T, F, true>::type(); }
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Bool, OP, LaunchPolicy::Blocking, I, F, true> {
	static typename OperateReturn<FailPolicy::Bool, OP, LaunchPolicy::Blocking, T, F, true>::type Operate(std::vector<typename I::value_type>&& id, F function) noexcept
	{
		static_assert(std::is_same<typename ObjectPolicyReturn<OP, T, F, true>::type, void>::value, "Function return value disregarded");

		auto reference = GameFactory::GetMultiple<T>(std::forward<I>(id));

		try
		{
			auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(reference); // depending on OP, returns either a reference or value
			function(param);
		}
		catch (...) { return false; }

		return true;
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Exception, OP, LaunchPolicy::Blocking, I, F, true> {
	static typename OperateReturn<FailPolicy::Exception, OP, LaunchPolicy::Blocking, T, F, true>::type Operate(std::vector<typename I::value_type>&& id, F function)
	{
		auto reference = GameFactory::GetMultiple<T>(std::forward<I>(id));
		auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(reference); // depending on OP, returns either a reference or value
		return function(param);
	}
};

template<typename T>
std::vector<FactoryWrapper<T>> GameFactory::GetObjectTypes(unsigned int type) noexcept
{
	std::vector<FactoryWrapper<T>> result;
	ReferenceList::iterator it;

	cs.StartSession();

	result.reserve(typecount[type]);
	ReferenceList copy = instances;

	cs.EndSession();

	for (const auto& reference : copy)
		if (reference.second & type)
		{
			auto object = FactoryWrapper<T>(reference.first.get(), reference.second);

			if (object)
				result.emplace_back(move(object));
		}

	return result;
}

template<typename T>
void GameFactory::LeaveReference(FactoryWrapper<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::LeaveReference Reference is NULL").stacktrace();

	_reference->EndSession();
	reference.reference = nullptr;
	reference.type = 0x00;
}

template<typename T>
RakNet::NetworkID GameFactory::CreateInstance()
{
	static_assert(std::is_base_of<Reference, T>::value, "T must be derived from Reference");

	std::shared_ptr<Reference> reference(new T());
	constexpr unsigned int type = rTypes<T>::value;

	RakNet::NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->SetBase(reference->GetBase());
#endif

	cs.StartSession();

	++typecount[type];
	// emplace
	index[id] = instances.insert(make_pair(reference, type)).first;

	cs.EndSession();

	return id;
}

template<typename T>
RakNet::NetworkID GameFactory::CreateKnownInstance(const pDefault* packet)
{
	static_assert(std::is_base_of<Reference, T>::value, "T must be derived from Reference");

	std::shared_ptr<Reference> reference(new T(packet));
	constexpr unsigned int type = rTypes<T>::value;

	RakNet::NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->SetBase(reference->GetBase());
#endif

	cs.StartSession();

	++typecount[type];
	// emplace
	index[id] = instances.insert(make_pair(reference, type)).first;

	cs.EndSession();

	return id;
}

template<typename T>
RakNet::NetworkID GameFactory::DestroyInstance(FactoryWrapper<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::DestroyInstance Reference is NULL").stacktrace();

	RakNet::NetworkID id = _reference->GetNetworkID();

#ifdef VAULTMP_DEBUG
	debug.print("Reference ", hex, _reference->GetReference(), " with base ",  _reference->GetBase(), " and NetworkID ", dec, _reference->GetNetworkID(), " (type: ", typeid(*_reference).name(), ") to be destructed");
#endif

	ReferenceList::key_type copy; // because the destructor of a type may also delete references, the actual destructor call must not happen within the CS block

	cs.StartSession();

	ReferenceList::iterator it = GetShared(id);

	copy = it->first; // saved. will be deleted past this block
	--typecount[it->second];
	_reference->Finalize();

	instances.erase(it);
	index.erase(id);
	delrefs.emplace(id);

	cs.EndSession();

	reference.reference = nullptr;
	reference.type = 0x00000000;

	return id;
}

template<>
class FactoryWrapper<Reference>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept;
		template<typename T, typename U>
		friend Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>& object) noexcept;

	private:
		Reference* reference;
		unsigned int type;

	protected:
		FactoryWrapper(Reference* reference, unsigned int type) noexcept : reference(static_cast<Reference*>(reference->StartSession())), type(type) {}

		FactoryWrapper(const FactoryWrapper& p) noexcept : reference(p.reference), type(p.type)
		{
			if (reference)
				reference->StartSession();
		}
		FactoryWrapper& operator=(const FactoryWrapper& p) noexcept
		{
			if (this != &p)
			{
				if (reference)
					reference->EndSession();

				reference = p.reference;
				type = p.type;

				if (reference)
					reference->StartSession();
			}

			return *this;
		}

		FactoryWrapper(FactoryWrapper&& p) noexcept : reference(p.reference), type(p.type)
		{
			p.reference = nullptr;
			p.type = 0x00000000;
		}
		FactoryWrapper& operator=(FactoryWrapper&& p) noexcept
		{
			if (this != &p)
			{
				if (reference)
					reference->EndSession();

				reference = p.reference;
				type = p.type;

				p.reference = nullptr;
				p.type = 0x00000000;
			}

			return *this;
		}

		FactoryWrapper() noexcept : reference(nullptr), type(0x00000000) {};
		~FactoryWrapper() noexcept
		{
			if (reference)
				reference->EndSession();
		}

	public:
		unsigned int GetType() const noexcept { return type; }
		Reference& operator*() const noexcept { return *reference; }
		Reference* operator->() const noexcept { return reference; }
		explicit operator bool() const noexcept { return reference; }
		bool operator==(const FactoryWrapper& p) const noexcept { return reference == p.reference; }
		bool operator!=(const FactoryWrapper& p) const noexcept { return !operator==(p); }

		template<typename T>
		inline bool validate(unsigned int type = 0x00000000) const noexcept;
};

#define GF_TYPE_WRAPPER(derived, base, token)                                                                                                                        \
	template<> inline bool FactoryWrapper<Reference>::validate<derived>(unsigned int type) const noexcept { return type ? (type & token) : (this->type & token); }   \
	template<> class FactoryWrapper<derived> : public FactoryWrapper<base>                                                                                           \
	{                                                                                                                                                                \
		friend class GameFactory;                                                                                                                                    \
                                                                                                                                                                     \
		template<typename T, typename U>                                                                                                                             \
		friend Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept;                                                                      \
		template<typename T, typename U>                                                                                                                             \
		friend Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>& object) noexcept;                                                                       \
                                                                                                                                                                     \
	protected:                                                                                                                                                       \
		FactoryWrapper(Reference* reference, unsigned int type) noexcept : FactoryWrapper<base>(reference, type)                                                     \
		{                                                                                                                                                            \
			if (!validate<derived>())                                                                                                                                \
				reference = nullptr;                                                                                                                                 \
		}                                                                                                                                                            \
		template<typename T> FactoryWrapper(const FactoryWrapper<T>& p) noexcept : FactoryWrapper<base>(p) {}                                                        \
		template<typename T> FactoryWrapper& operator=(const FactoryWrapper<T>& p) noexcept { return FactoryWrapper<base>::operator=(p); }                           \
                                                                                                                                                                     \
	public:                                                                                                                                                          \
		FactoryWrapper() noexcept : FactoryWrapper<base>() {}                                                                                                        \
		FactoryWrapper(const FactoryWrapper& p) noexcept : FactoryWrapper<base>(p) {}                                                                                \
		FactoryWrapper& operator=(const FactoryWrapper&) = default;                                                                                                  \
		FactoryWrapper(FactoryWrapper&& p) noexcept : FactoryWrapper<base>(std::move(p)) {}                                                                          \
		FactoryWrapper& operator=(FactoryWrapper&&) = default;                                                                                                       \
		~FactoryWrapper() = default;                                                                                                                                 \
																																							         \
		derived* operator->() const noexcept { return static_cast<derived*>(FactoryWrapper<Reference>::operator->()); }                                              \
		derived& operator*() const noexcept { return static_cast<derived&>(FactoryWrapper<Reference>::operator*()); }                                                \
};                                                                                                                                                                   \
typedef FactoryWrapper<derived> Factory##derived;                                                                                                                    \
typedef std::vector<FactoryWrapper<derived>> Factory##derived##s;                                                                                                    \
typedef Expected<FactoryWrapper<derived>> Expected##derived;                                                                                                         \
typedef std::vector<Expected<FactoryWrapper<derived>>> Expected##derived##s;

/**
  * \brief Tries to cast the instance pointer of a FactoryWrapper
  */
template<typename T, typename U>
inline Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept
{
	if (!object.template validate<T>())
		return VaultException("vaultcast failed");

	return FactoryWrapper<T>(object);
}
template<typename T, typename U>
inline Expected<FactoryWrapper<T>> vaultcast(const Expected<FactoryWrapper<U>>& object) noexcept { return vaultcast<T>(const_cast<Expected<FactoryWrapper<U>>&>(object).get()); }

template<typename T, typename U>
inline Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>& object) noexcept
{
	if (!object.template validate<T>())
		return VaultException("vaultcast failed");

	auto result = FactoryWrapper<T>(object);

	GameFactory::LeaveReference(object);

	return result;
}

template<typename T, typename U> inline Expected<FactoryWrapper<T>> vaultcast_swap(Expected<FactoryWrapper<U>>& object) noexcept { return vaultcast_swap<T>(object.get()); }
template<typename T, typename U> inline bool vaultcast_test(const FactoryWrapper<U>& object) noexcept { return object.template validate<T>(); }
template<typename T, typename U> inline bool vaultcast_test(const Expected<FactoryWrapper<U>>& object) noexcept { return const_cast<Expected<FactoryWrapper<U>>&>(object).get().template validate<T>(); }

#endif
