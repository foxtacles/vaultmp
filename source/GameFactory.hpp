#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

#include "vaultmp.hpp"
#include "Base.hpp"
#include "Expected.hpp"
#include "Guarded.hpp"

#ifdef VAULTSERVER
#include "vaultserver/Database.hpp"
#include "vaultserver/Record.hpp"
#include "vaultserver/Reference.hpp"
#include "vaultserver/Exterior.hpp"
#include "vaultserver/Weapon.hpp"
#include "vaultserver/Race.hpp"
#include "vaultserver/NPC.hpp"
#include "vaultserver/BaseContainer.hpp"
#include "vaultserver/Item.hpp"
#include "vaultserver/Terminal.hpp"
#include "vaultserver/Interior.hpp"
#include "vaultserver/ActorReference.hpp"
#include "vaultserver/CreatureReference.hpp"
#endif

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <map>
#include <memory>
#include <unordered_set>

/**
  * \brief Holds an instance pointer
  */
template<typename T>
class FactoryWrapper;

template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept;
template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast(const Expected<FactoryWrapper<U>>& object) noexcept;
template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>&& object) noexcept;
template<typename T, typename U> Expected<FactoryWrapper<T>> vaultcast_swap(Expected<FactoryWrapper<U>>&& object) noexcept;

template<typename T> struct rTypes;
template<typename T> struct rTypesToken;

/**
 * \brief Create, use and destroy game object instances via the GameFactory
 */
class GameFactory
{
	private:
		typedef std::map<std::shared_ptr<Base>, unsigned int> BaseList;
		typedef std::unordered_map<RakNet::NetworkID, BaseList::iterator> BaseIndex;
		typedef std::unordered_map<unsigned int, unsigned int> BaseCount;
		typedef std::unordered_set<RakNet::NetworkID> BaseDeleted;

		GameFactory() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<GameFactory> debug;
#endif

		static Guarded<> cs;
		static BaseList instances;
		static BaseIndex index;
		static BaseCount typecount;
		static BaseDeleted delrefs;

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
		static Database<DB::ActorReference> dbActorReferences;
		static Database<DB::CreatureReference> dbCreatureReferences;
#endif

		inline static BaseList::iterator GetShared(const RakNet::NetworkID& id) { return index.count(id) ? index[id] : instances.end(); }

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
		template<ObjectPolicy OP, typename T> using OPT = typename ObjectPolicyType<OP, T>::type;

		template<ObjectPolicy OP, typename T, typename F, bool M> struct ObjectPolicyReturn;
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Validated, T, F, false> { typedef typename std::result_of<F(FactoryWrapper<T>&)>::type type; };
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Expected, T, F, false> { typedef typename std::result_of<F(Expected<FactoryWrapper<T>>&)>::type type; };
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Validated, T, F, true> { typedef typename std::result_of<F(std::vector<FactoryWrapper<T>>&)>::type type; };
		template<typename T, typename F> struct ObjectPolicyReturn<ObjectPolicy::Expected, T, F, true> { typedef typename std::result_of<F(std::vector<Expected<FactoryWrapper<T>>>&)>::type type; };
		template<ObjectPolicy OP, typename T, typename F, bool M> using OPR = typename ObjectPolicyReturn<OP, T, F, M>::type;

		template<typename I>
		struct InputPolicyHelper {
			static constexpr bool value = !std::is_trivial<typename std::remove_reference<I>::type>::value && !std::is_base_of<FactoryWrapper<Base>, typename std::remove_reference<I>::type>::value;
		};

		template<FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename T, typename F, typename I> struct OperateReturn;
		template<ObjectPolicy OP, typename T, typename F, typename I> struct OperateReturn<FailPolicy::Bool, OP, LaunchPolicy::Blocking, T, F, I> { typedef bool type; };
		template<typename T, typename F, typename I> struct OperateReturn<FailPolicy::Return, ObjectPolicy::Validated, LaunchPolicy::Blocking, T, F, I> { typedef OPR<ObjectPolicy::Validated, T, F, InputPolicyHelper<I>::value> type; };
		template<typename T, typename F, typename I> struct OperateReturn<FailPolicy::Return, ObjectPolicy::Expected, LaunchPolicy::Blocking, T, F, I> { typedef OPR<ObjectPolicy::Expected, T, F, InputPolicyHelper<I>::value> type; };
		template<typename T, typename F, typename I> struct OperateReturn<FailPolicy::Exception, ObjectPolicy::Validated, LaunchPolicy::Blocking, T, F, I> { typedef OPR<ObjectPolicy::Validated, T, F, InputPolicyHelper<I>::value> type; };
		template<typename T, typename F, typename I> struct OperateReturn<FailPolicy::Exception, ObjectPolicy::Expected, LaunchPolicy::Blocking, T, F, I> { typedef OPR<ObjectPolicy::Expected, T, F, InputPolicyHelper<I>::value> type; };
		template<FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename T, typename F, typename I> using OR = typename OperateReturn<FP, OP, LP, T, F, I>::type;

		template<typename T, FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename I, typename F>
		struct OperateFunctions {
			static typename OperateReturn<FP, OP, LP, T, F, I>::type Operate(I&& id, F function) noexcept(FP != FailPolicy::Exception);
		};

		template<ObjectPolicy OP, typename T>
		struct ObjectPolicyHelper {
			inline static OPT<OP, T>& Unwrap(Expected<FactoryWrapper<T>>& base) noexcept(OP == ObjectPolicy::Expected);
			inline static OPT<OP, T>& Unwrap(std::vector<Expected<FactoryWrapper<T>>>& base) noexcept(OP == ObjectPolicy::Expected);
		};

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::Validated, T> {
			inline static OPT<ObjectPolicy::Validated, T>& Unwrap(Expected<FactoryWrapper<T>>& base) { return base.get(); }
			inline static typename std::vector<OPT<ObjectPolicy::Validated, T>> Unwrap(std::vector<Expected<FactoryWrapper<T>>>& base) {
				// a solution with std::reference_wrapper would also be possible. not sure which would perform better
				std::vector<FactoryWrapper<T>> result;
				result.reserve(base.size());

				for (auto& base_ : base)
					result.emplace_back(std::move(base_.get()));

				return result;
			}
		};

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::Expected, T> {
			inline static OPT<ObjectPolicy::Expected, T>& Unwrap(Expected<FactoryWrapper<T>>& base) noexcept { return base; }
			inline static std::vector<OPT<ObjectPolicy::Expected, T>>& Unwrap(std::vector<Expected<FactoryWrapper<T>>>& base) noexcept { return base; }
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
		 * \brief Obtains a lock on a Base
		 */
		template<typename T, typename I = RakNet::NetworkID>
		inline static typename std::enable_if<std::is_trivial<I>::value, Expected<FactoryWrapper<T>>>::type Get(I id) noexcept { return Get_<T, I>::Get(id); }
		/**
		 * \brief This is an alias to vaultcast_swap
		 */
		template<typename T, typename U> inline static Expected<FactoryWrapper<T>> Get(FactoryWrapper<U>&& base) noexcept { return vaultcast_swap<T>(move(base)); }
		template<typename T, typename U> inline static Expected<FactoryWrapper<T>> Get(Expected<FactoryWrapper<U>>&& base) noexcept { return vaultcast_swap<T>(move(base)); }
		/**
		 * \brief Obtains a lock on multiple Bases
		 */
		template<typename T, typename I = RakNet::NetworkID>
		inline static std::vector<Expected<FactoryWrapper<T>>> Get(const std::vector<I>& ids) noexcept { return Get_<T, I>::Get(ids); }
		/**
		 * \brief Executes a function on one or multiple Bases
		 */
		template<typename T, FailPolicy FP = FailPolicy::Default, ObjectPolicy OP = ObjectPolicy::Default, LaunchPolicy LP = LaunchPolicy::Default, typename I, typename F>
		static OR<FP, OP, LP, T, F, I> Operate(I&& id, F function) noexcept(FP != FailPolicy::Exception) { return OperateFunctions<T, FP, OP, LP, I, F>::Operate(std::forward<I>(id), function); }
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
		 * \brief Returns the type of the given base
		 */
		static unsigned int GetType(const Base* base) noexcept;
		/**
		 * \brief Returns the NetworkID's of all Bases of a given type
		 */
		static std::vector<RakNet::NetworkID> GetByType(unsigned int type) noexcept;
		/**
		 * \brief Counts the amount of Bases of a given type
		 */
		static unsigned int GetCount(unsigned int type) noexcept;
		/**
		 * \brief Invalidates a Base held by a FactoryWrapper
		 */
		template<typename T>
		static void Leave(FactoryWrapper<T>& base);
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
		 * You must make sure the lock count of the given Base equals to one
		 */
		template<typename T>
		static RakNet::NetworkID Destroy(FactoryWrapper<T>& base);
};

template<typename T>
struct GameFactory::Get_<T, RakNet::NetworkID> {
	static Expected<FactoryWrapper<T>> Get(RakNet::NetworkID id) noexcept
	{
		std::pair<BaseList::key_type, BaseList::mapped_type> base;

		cs.Operate([id, &base]() {
			auto it = GetShared(id);

			if (it != instances.end())
				base = *it;
		});

		if (!base.first)
			return VaultException("Unknown object with NetworkID %llu", id);

		return FactoryWrapper<T>(base.first.get(), base.second);
	}

	static std::vector<Expected<FactoryWrapper<T>>> Get(const std::vector<RakNet::NetworkID>& ids) noexcept
	{
		std::vector<Expected<FactoryWrapper<T>>> result(ids.size());
		std::multimap<BaseList::value_type, unsigned int> sort;

		cs.Operate([&ids, &result, &sort]() {
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
		});

		for (const auto& base : sort)
			result[base.second] = FactoryWrapper<T>(base.first.first.get(), base.first.second);

		return result;
	}
};

using FailPolicy = GameFactory::FailPolicy;
using LaunchPolicy = GameFactory::LaunchPolicy;
using ObjectPolicy = GameFactory::ObjectPolicy;

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Return, OP, LaunchPolicy::Blocking, I, F> {
	static OR<FailPolicy::Return, OP, LaunchPolicy::Blocking, T, F, I> Operate(I&& id, F function) noexcept
	{
		auto base = GameFactory::Get<T>(std::forward<I>(id));

		try
		{
			auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
			return function(param);
		}
		catch (...) { return OPR<OP, T, F, InputPolicyHelper<I>::value>(); }
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Bool, OP, LaunchPolicy::Blocking, I, F> {
	static OR<FailPolicy::Bool, OP, LaunchPolicy::Blocking, T, F, I> Operate(I&& id, F function) noexcept
	{
		static_assert(std::is_same<OPR<OP, T, F, InputPolicyHelper<I>::value>, void>::value, "Function return value disregarded");

		auto base = GameFactory::Get<T>(std::forward<I>(id));

		try
		{
			auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
			function(param);
		}
		catch (...) { return false; }

		return true;
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Exception, OP, LaunchPolicy::Blocking, I, F> {
	static OR<FailPolicy::Exception, OP, LaunchPolicy::Blocking, T, F, I> Operate(I&& id, F function)
	{
		auto base = GameFactory::Get<T>(std::forward<I>(id));
		auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
		return function(param);
	}
};

template<typename T>
void GameFactory::Leave(FactoryWrapper<T>& base)
{
	Base* _base = base.base;

	if (!_base)
		throw VaultException("GameFactory::Leave Base is NULL").stacktrace();

	_base->EndSession();
	base.base = nullptr;
	base.type = 0x00000000;
}

template<typename T, typename... Args>
RakNet::NetworkID GameFactory::Create(Args&&... args)
{
	static_assert(std::is_base_of<Base, T>::value, "T must be derived from Base");

	std::shared_ptr<Base> base(new T(std::forward<Args>(args)...));
	constexpr unsigned int type = rTypes<T>::value;

	RakNet::NetworkID id = base->GetNetworkID();

#ifdef VAULTSERVER
	base->initializers();
#endif

	cs.Operate([id, type, &base]() {
		++typecount[type];
		// emplace
		index[id] = instances.insert(make_pair(std::move(base), type)).first;
	});

	return id;
}

template<typename T>
RakNet::NetworkID GameFactory::Create(const pDefault* packet)
{
	static_assert(std::is_base_of<Base, T>::value, "T must be derived from Base");

	std::shared_ptr<Base> base(new T(packet));
	constexpr unsigned int type = rTypes<T>::value;

	RakNet::NetworkID id = base->GetNetworkID();

	cs.Operate([id, type, &base]() {
		++typecount[type];
		// emplace
		index[id] = instances.insert(make_pair(std::move(base), type)).first;
	});

	return id;
}

template<typename T>
RakNet::NetworkID GameFactory::Destroy(FactoryWrapper<T>& base)
{
	Base* _base = base.base;

	if (!_base)
		throw VaultException("GameFactory::Destroy Base is NULL").stacktrace();

	RakNet::NetworkID id = _base->GetNetworkID();

#ifdef VAULTMP_DEBUG
	debug.print("Base ", std::dec, _base->GetNetworkID(), " (type: ", typeid(*_base).name(), ") to be destructed");
#endif

	BaseList::key_type copy; // because the destructor of a type may also delete bases, the actual destructor call must not happen within the CS block

	cs.Operate([id, _base, &copy]() {
		auto it = GetShared(id);

		copy = it->first; // saved. will be deleted past this block
		--typecount[it->second];
		_base->Finalize();

		instances.erase(it);
		index.erase(id);
		delrefs.emplace(id);
	});

	base.base = nullptr;
	base.type = 0x00000000;

	return id;
}

template<>
class FactoryWrapper<Base>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept;
		template<typename T, typename U>
		friend Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>&& object) noexcept;

	protected:
		Base* base;
		unsigned int type;

		FactoryWrapper(Base* base, unsigned int type) noexcept : base(base ? static_cast<Base*>(base->StartSession()) : nullptr), type(type) {}

	public:
		FactoryWrapper(const FactoryWrapper& p) noexcept : base(p.base), type(p.type)
		{
			if (base)
				base->StartSession();
		}
		FactoryWrapper& operator=(const FactoryWrapper& p) noexcept
		{
			if (this != &p)
			{
				if (base)
					base->EndSession();

				base = p.base;
				type = p.type;

				if (base)
					base->StartSession();
			}

			return *this;
		}

		FactoryWrapper(FactoryWrapper&& p) noexcept : base(p.base), type(p.type)
		{
			p.base = nullptr;
			p.type = 0x00000000;
		}
		FactoryWrapper& operator=(FactoryWrapper&& p) noexcept
		{
			if (this != &p)
			{
				if (base)
					base->EndSession();

				base = p.base;
				type = p.type;

				p.base = nullptr;
				p.type = 0x00000000;
			}

			return *this;
		}

		FactoryWrapper() noexcept : base(nullptr), type(0x00000000) {};
		~FactoryWrapper() noexcept
		{
			if (base)
				base->EndSession();
		}

		unsigned int GetType() const noexcept { return type; }
		Base& operator*() const noexcept { return *base; }
		Base* operator->() const noexcept { return base; }
		explicit operator bool() const noexcept { return base; }
		bool operator==(const FactoryWrapper& p) const noexcept { return base == p.base; }
		bool operator!=(const FactoryWrapper& p) const noexcept { return !operator==(p); }

		template<typename T>
		inline bool validate(unsigned int type = 0x00000000) const noexcept { return type ? (type & rTypesToken<T>::value) : (this->type & rTypesToken<T>::value); }
};
typedef FactoryWrapper<Base> FactoryBase;
typedef std::vector<FactoryWrapper<Base>> FactoryBases;
typedef Expected<FactoryWrapper<Base>> ExpectedBase;
typedef std::vector<Expected<FactoryWrapper<Base>>> ExpectedBases;

#define GF_TYPE_WRAPPER(derived_class, base_class, identity, token)                                                                                                  \
	template<> class FactoryWrapper<derived_class> : public FactoryWrapper<base_class>                                                                               \
	{                                                                                                                                                                \
		friend class GameFactory;                                                                                                                                    \
                                                                                                                                                                     \
		template<typename T, typename U>                                                                                                                             \
		friend Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept;                                                                      \
		template<typename T, typename U>                                                                                                                             \
		friend Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>&& object) noexcept;                                                                      \
                                                                                                                                                                     \
	protected:                                                                                                                                                       \
		FactoryWrapper(Base* base, unsigned int type) noexcept : FactoryWrapper<base_class>(validate<derived_class>(type) ? base : nullptr, type) {}                 \
		template<typename T> FactoryWrapper(const FactoryWrapper<T>& p) noexcept : FactoryWrapper<base_class>(p) {}                                                  \
		template<typename T> FactoryWrapper& operator=(const FactoryWrapper<T>& p) noexcept { return FactoryWrapper<base_class>::operator=(p); }                     \
                                                                                                                                                                     \
	public:                                                                                                                                                          \
		FactoryWrapper() noexcept : FactoryWrapper<base_class>() {}                                                                                                  \
		FactoryWrapper(const FactoryWrapper& p) noexcept : FactoryWrapper<base_class>(p) {}                                                                          \
		FactoryWrapper& operator=(const FactoryWrapper&) = default;                                                                                                  \
		FactoryWrapper(FactoryWrapper&& p) noexcept : FactoryWrapper<base_class>(std::move(p)) {}                                                                    \
		FactoryWrapper& operator=(FactoryWrapper&&) = default;                                                                                                       \
		~FactoryWrapper() = default;                                                                                                                                 \
																																							         \
		derived_class* operator->() const noexcept { return dynamic_cast<derived_class*>(base); }                                                                    \
		derived_class& operator*() const noexcept { return dynamic_cast<derived_class&>(*base); }                                                                    \
};                                                                                                                                                                   \
template<> struct rTypes<derived_class> { enum { value = identity }; };                                                                                              \
template<> struct rTypesToken<derived_class> { enum { value = token }; };                                                                                            \
typedef FactoryWrapper<derived_class> Factory##derived_class;                                                                                                        \
typedef std::vector<FactoryWrapper<derived_class>> Factory##derived_class##s;                                                                                        \
typedef Expected<FactoryWrapper<derived_class>> Expected##derived_class;                                                                                             \
typedef std::vector<Expected<FactoryWrapper<derived_class>>> Expected##derived_class##s;

#define GF_TYPE_WRAPPER_FINAL(derived_class, base_class, identity) GF_TYPE_WRAPPER(derived_class, base_class, identity, identity)

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
inline Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>&& object) noexcept
{
	if (!object.template validate<T>())
		return VaultException("vaultcast failed");

	return FactoryWrapper<T>(std::move(object));
}

template<typename T, typename U> inline Expected<FactoryWrapper<T>> vaultcast_swap(Expected<FactoryWrapper<U>>&& object) noexcept { return vaultcast_swap<T>(std::move(object.get())); }
template<typename T, typename U> inline bool vaultcast_test(const FactoryWrapper<U>& object) noexcept { return object.template validate<T>(); }
template<typename T, typename U> inline bool vaultcast_test(const Expected<FactoryWrapper<U>>& object) noexcept { return const_cast<Expected<FactoryWrapper<U>>&>(object).get().template validate<T>(); }

#endif
