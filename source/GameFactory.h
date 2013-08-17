#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

#include "vaultmp.h"
#include "Reference.h"
#include "Expected.h"
#include "Guarded.h"

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
template<typename T> struct rTypesToken;

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

		static Guarded<> cs;
		static ReferenceList instances;
		static ReferenceIndex index;
		static ReferenceCount typecount;
		static ReferenceDeleted delrefs;

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
					GameFactory::Leave(reference__);
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

		template<typename T, typename I>
		struct Get_ {
			inline static Expected<FactoryWrapper<T>> Get(I id) noexcept {
				return Get_<T, RakNet::NetworkID>::Get(T::template PickBy<I>(id));
			}
			inline static std::vector<Expected<FactoryWrapper<T>>> Get(const std::vector<I>& ids) noexcept {
				return Get_<T, RakNet::NetworkID>::Get(T::template PickBy<I>(ids));
			}
		};

	public:
		static void Initialize();

		/**
		 * \brief Obtains a lock on a Reference
		 *
		 * The Reference is identified by a NetworkID
		 */
		template<typename T, typename I = RakNet::NetworkID>
		inline static typename std::enable_if<std::is_trivial<I>::value, Expected<FactoryWrapper<T>>>::type Get(I id) noexcept { return Get_<T, I>::Get(id); }
		/**
		 * \brief This is an alias to vaultcast_swap
		 */
		template<typename T, typename U> inline static Expected<FactoryWrapper<T>> Get(FactoryWrapper<U>& reference) noexcept { return vaultcast_swap<T>(reference); }
		template<typename T, typename U> inline static Expected<FactoryWrapper<T>> Get(Expected<FactoryWrapper<U>>& reference) noexcept { return vaultcast_swap<T>(reference); }
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of NetworkID. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		template<typename T, typename I = RakNet::NetworkID>
		inline static std::vector<Expected<FactoryWrapper<T>>> Get(const std::vector<I>& ids) noexcept { return Get_<T, I>::Get(ids); }
		/**
		 * \brief Executes a function on one or multiple References
		 *
		 * The References are identified by an arbitrary type. This can be a NetworkID, reference ID or FactoryWrapper
		 */
		template<typename T, FailPolicy FP = FailPolicy::Default, ObjectPolicy OP = ObjectPolicy::Default, LaunchPolicy LP = LaunchPolicy::Default, typename I, typename F>
		static typename OperateReturn<FP, OP, LP, T, F, InputPolicyHelper<I>::M>::type Operate(I&& id, F function) noexcept(FP != FailPolicy::Exception) { return OperateFunctions<T, FP, OP, LP, I, F, InputPolicyHelper<I>::M>::Operate(std::forward<I>(id), function); }
		/**
		 * \brief Lookup an ID
		 */
		template<typename T, typename I>
		inline static RakNet::NetworkID Lookup(I id) noexcept { return T::template PickBy<I>(id); }
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
		template<typename T>
		static std::vector<FactoryWrapper<T>> GetByType(unsigned int type) noexcept;
		/**
		 * \brief Returns the NetworkID's of all References of a given type
		 */
		static std::vector<RakNet::NetworkID> GetByTypeID(unsigned int type) noexcept;
		/**
		 * \brief Counts the amount of References of a given type
		 */
		static unsigned int GetCount(unsigned int type) noexcept;
		/**
		 * \brief Invalidates a Reference held by a FactoryWrapper
		 */
		template<typename T>
		static void Leave(FactoryWrapper<T>& reference);
		/**
		 * \brief Creates a new instance of a given type
		 */
		template<typename T, typename... Args>
		static RakNet::NetworkID Create(Args&&... args);
		/**
		 * \brief Creates a known instance from a network packet
		 */
		template<typename T>
		static RakNet::NetworkID Create(const pDefault* packet);

		/**
		 * \brief Destroys all instances and cleans up type classes
		 */
		static void DestroyAll() noexcept;
		/**
		 * \brief Destroys an instance
		 */
		static bool Destroy(RakNet::NetworkID id);
		/**
		 * \brief Destroys an instance which has previously been locked
		 *
		 * You must make sure the lock count of the given Reference equals to one
		 */
		template<typename T>
		static RakNet::NetworkID Destroy(FactoryWrapper<T>& reference);
};

#include "GameFactory.hpp"

#endif
