#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

#include "vaultmp.hpp"
#include "Base.hpp"
#include "Expected.hpp"
#include "Guarded.hpp"
#include "Utils.hpp"

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
#include "vaultserver/AcReference.hpp"
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

template<typename T> struct rBases;
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
		static Database<DB::AcReference> dbAcReferences;
#endif

		inline static BaseList::iterator GetShared(RakNet::NetworkID id) { return index.count(id) ? index[id] : instances.end(); }

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
			FactoryValidated,
			FactoryExpected,
			Default = Validated
		};

		#define RETURN_VALIDATED             FailPolicy::Return, ObjectPolicy::Validated
		#define RETURN_EXPECTED              FailPolicy::Return, ObjectPolicy::Expected
		#define RETURN_FACTORY_VALIDATED     FailPolicy::Return, ObjectPolicy::FactoryValidated
		#define RETURN_FACTORY_EXPECTED      FailPolicy::Return, ObjectPolicy::FactoryExpected
		#define EXCEPTION_VALIDATED          FailPolicy::Exception, ObjectPolicy::Validated
		#define EXCEPTION_EXPECTED           FailPolicy::Exception, ObjectPolicy::Expected
		#define EXCEPTION_FACTORY_VALIDATED  FailPolicy::Exception, ObjectPolicy::FactoryValidated
		#define EXCEPTION_FACTORY_EXPECTED   FailPolicy::Exception, ObjectPolicy::FactoryExpected
		#define BOOL_VALIDATED               FailPolicy::Bool, ObjectPolicy::Validated
		#define BOOL_EXPECTED                FailPolicy::Bool, ObjectPolicy::Expected
		#define BOOL_FACTORY_VALIDATED       FailPolicy::Bool, ObjectPolicy::FactoryValidated
		#define BOOL_FACTORY_EXPECTED        FailPolicy::Bool, ObjectPolicy::FactoryExpected

	private:
		template<typename T, FailPolicy FP, ObjectPolicy OP, LaunchPolicy LP, typename I, typename F>
		struct OperateFunctions;

		template<ObjectPolicy OP, typename T>
		struct ObjectPolicyHelper;

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::Validated, T> {
			inline static auto Unwrap(Expected<FactoryWrapper<T>>& base) { return base.get().operator->(); }
			inline static auto Unwrap(std::vector<Expected<FactoryWrapper<T>>>& base) {
				std::vector<T*> result;
				result.reserve(base.size());

				for (auto& base_ : base)
					result.emplace_back(base_.get().operator->());

				return result;
			}
		};

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::Expected, T> {
			inline static auto Unwrap(Expected<FactoryWrapper<T>>& base) noexcept { return base ? base.get().operator->() : nullptr; }
			inline static auto Unwrap(std::vector<Expected<FactoryWrapper<T>>>& base) noexcept {
				std::vector<T*> result;
				result.reserve(base.size());

				for (auto& base_ : base)
					result.emplace_back(base_ ? base_.get().operator->() : nullptr);

				return result;
			}
		};

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::FactoryValidated, T> {
			inline static auto& Unwrap(Expected<FactoryWrapper<T>>& base) { return base.get(); }
			inline static auto Unwrap(std::vector<Expected<FactoryWrapper<T>>>& base) {
				std::vector<FactoryWrapper<T>> result;
				result.reserve(base.size());

				for (auto& base_ : base)
					result.emplace_back(std::move(base_.get()));

				return result;
			}
		};

		template<typename T>
		struct ObjectPolicyHelper<ObjectPolicy::FactoryExpected, T> {
			inline static auto& Unwrap(Expected<FactoryWrapper<T>>& base) noexcept { return base; }
			inline static auto& Unwrap(std::vector<Expected<FactoryWrapper<T>>>& base) noexcept { return base; }
		};

		template<typename T, typename I>
		struct Get_ {
			inline static auto Get(I id) noexcept {
				return Get_<T, RakNet::NetworkID>::Get(T::template PickBy<I>(id));
			}
			template<template<typename...> class C>
			inline static auto Get(C<I>&& ids) noexcept {
				return Get_<T, RakNet::NetworkID>::Get(T::template PickBy<I>(std::move(ids)));
			}
			template<template<typename...> class C>
			inline static auto Get(const C<I>& ids) noexcept {
				return Get_<T, RakNet::NetworkID>::Get(T::template PickBy<I>(ids));
			}
		};

		template<typename T, FailPolicy FP, typename... Args>
		struct Create_;

	public:
		static void Initialize();

		/**
		 * \brief Obtains a lock on a Base
		 */
		template<typename T, typename I>
		inline static typename std::enable_if<std::is_trivial<I>::value, Expected<FactoryWrapper<T>>>::type Get(I id) noexcept { return Get_<T, I>::Get(id); }
		/**
		 * \brief This is an alias to vaultcast_swap
		 */
		template<typename T, typename U> inline static auto Get(FactoryWrapper<U>&& base) noexcept { return vaultcast_swap<T>(std::move(base)); }
		template<typename T, typename U> inline static auto Get(Expected<FactoryWrapper<U>>&& base) noexcept { return vaultcast_swap<T>(std::move(base)); }
		/**
		 * \brief Obtains a lock on multiple Bases
		 */
		template<typename T, template<typename...> class C, typename I> inline static std::vector<Expected<FactoryWrapper<T>>> Get(C<I>&& ids) noexcept { return Get_<T, I>::Get(std::move(ids)); }
		template<typename T, template<typename...> class C, typename I> inline static std::vector<Expected<FactoryWrapper<T>>> Get(const C<I>& ids) noexcept { return Get_<T, I>::Get(ids); }
		template<typename T, typename I> inline static std::vector<Expected<FactoryWrapper<T>>> Get(std::initializer_list<I>&& ids) noexcept { return Get_<T, I>::Get(std::move(ids)); }
		/**
		 * \brief Executes a function on one or multiple Bases
		 */
		template<typename T, FailPolicy FP = FailPolicy::Default, ObjectPolicy OP = ObjectPolicy::Default, LaunchPolicy LP = LaunchPolicy::Default, typename I, typename F>
		static auto Operate(I&& id, F function) noexcept(FP != FailPolicy::Exception) { return OperateFunctions<T, FP, OP, LP, I, F>::Operate(std::forward<I>(id), function); }
		template<typename T, FailPolicy FP = FailPolicy::Default, ObjectPolicy OP = ObjectPolicy::Default, LaunchPolicy LP = LaunchPolicy::Default, typename I, typename F>
		static auto Operate(std::initializer_list<I>&& id, F function) noexcept(FP != FailPolicy::Exception) { return OperateFunctions<T, FP, OP, LP, std::initializer_list<I>, F>::Operate(std::forward<std::initializer_list<I>>(id), function); }
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
		 * \brief Returns true if the given object exists
		 */
		template<typename T>
		static bool Exists(RakNet::NetworkID id) noexcept { return GetType(id) & rTypesToken<T>::value; }
		static bool Exists(RakNet::NetworkID id) noexcept { return GetType(id); }
		/**
		 * \brief Returns true if the type matches exactly
		 */
		template<typename T>
		static bool Is(RakNet::NetworkID id) noexcept { return GetType(id) == rTypes<T>::value; }
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
		static void Free(FactoryWrapper<T>& base);
		/**
		 * \brief Creates a new instance of a given type
		 */
		template<typename T, FailPolicy FP, typename... Args>
		static auto Create(Args&&... args) { return Create_<T, FP, Args...>::Create(std::forward<Args>(args)...); }

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

	template<template<typename...> class C>
	static auto Get(C<RakNet::NetworkID>&& ids) noexcept { return Get(ids); }

	template<template<typename...> class C>
	static auto Get(const C<RakNet::NetworkID>& ids) noexcept
	{
		std::vector<Expected<FactoryWrapper<T>>> result(ids.size());
		std::multimap<BaseList::value_type, unsigned int> sort;

		cs.Operate([&ids, &result, &sort]() {
			unsigned int i = 0;

			for (auto id : ids)
			{
				auto it = GetShared(id);

				if (it == instances.end())
					result[i] = VaultException("Unknown object with NetworkID %llu", id);
				else
					sort.emplace(*it, i);

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
	static auto Operate(I&& id, F function) noexcept
	{
		auto base = GameFactory::Get<T>(std::forward<I>(id));

		try
		{
			auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
			return function(param);
		}
		catch (...)
		{
			using return_type = typename std::remove_reference<decltype(ObjectPolicyHelper<OP, T>::Unwrap(base))>::type;
			return_type* R;
			return decltype(function(*R))();
		}
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Bool, OP, LaunchPolicy::Blocking, I, F> {
	static auto Operate(I&& id, F function) noexcept
	{
		auto base = GameFactory::Get<T>(std::forward<I>(id));

		try
		{
			auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
			function(param);

			static_assert(std::is_same<decltype(function(param)), void>::value, "Function return value disregarded");
		}
		catch (...) { return false; }

		return true;
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Exception, OP, LaunchPolicy::Blocking, I, F> {
	static auto Operate(I&& id, F function)
	{
		auto base = GameFactory::Get<T>(std::forward<I>(id));
		auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
		return function(param);
	}
};

template<typename T>
void GameFactory::Free(FactoryWrapper<T>& base)
{
	Base* _base = base.base;

	if (!_base)
		throw VaultException("GameFactory::Free Base is NULL").stacktrace();

	_base->EndSession();
	base.base = nullptr;
	base.type = 0x00000000;
}

template<typename T, typename... Args>
struct GameFactory::Create_<T, FailPolicy::Exception, Args...> {
	static RakNet::NetworkID Create(Args&&... args)
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
			index[id] = instances.emplace(std::move(base), type).first;
		});

		return id;
	}
};

template<typename T, typename... Args>
struct GameFactory::Create_<T, FailPolicy::Return, Args...> {
	static RakNet::NetworkID Create(Args&&... args)
	{
		try
		{
			return Create_<T, FailPolicy::Exception, Args...>::Create(std::forward<Args>(args)...);
		}
		catch (...) { return 0ull; }
	}
};

template<typename T, typename... Args>
struct GameFactory::Create_<T, FailPolicy::Bool, Args...> {
	static bool Create(Args&&... args)
	{
		return Create_<T, FailPolicy::Return, Args...>::Create(std::forward<Args>(args)...);
	}
};

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

template<typename T>
class FactoryWrapperPtrType
{
	private:
		T* ptr;

	protected:
		FactoryWrapperPtrType() : ptr(nullptr) {}
		FactoryWrapperPtrType(T* ptr) : ptr(ptr) {}
		//FactoryWrapperPtrType(Base* ptr) : ptr(ptr ? dynamic_cast<T*>(ptr) : nullptr) {}

		inline T* get_ptr(Base*) const { return ptr; }

	public:
		FactoryWrapperPtrType(const FactoryWrapperPtrType&) = default;
		FactoryWrapperPtrType& operator=(const FactoryWrapperPtrType&) = default;
		FactoryWrapperPtrType(FactoryWrapperPtrType&& p) noexcept : ptr(p.ptr) { p.ptr = nullptr; }
		FactoryWrapperPtrType& operator=(FactoryWrapperPtrType&& p) noexcept
		{
			if (this != &p)
			{
				ptr = p.ptr;
				p.ptr = nullptr;
			}

			return *this;
		}
		~FactoryWrapperPtrType() = default;
};

template<typename T>
class FactoryWrapperPtrEmpty
{
	protected:
		FactoryWrapperPtrEmpty() {}
		FactoryWrapperPtrEmpty(Base*) {}

		inline T* get_ptr(Base* base) const { return static_cast<T*>(base); }

	public:
		FactoryWrapperPtrEmpty(const FactoryWrapperPtrEmpty&) = default;
		FactoryWrapperPtrEmpty& operator=(const FactoryWrapperPtrEmpty&) = default;
		FactoryWrapperPtrEmpty(FactoryWrapperPtrEmpty&&) = default;
		FactoryWrapperPtrEmpty& operator=(FactoryWrapperPtrEmpty&&) = default;
		~FactoryWrapperPtrEmpty() = default;
};

template<typename F, typename T>
using FactoryWrapperPtr = typename std::conditional<Utils::is_static_castable<F*, T*>::value, FactoryWrapperPtrEmpty<T>, FactoryWrapperPtrType<T>>::type;

template<>
class FactoryWrapper<Base> : protected FactoryWrapperPtr<Base, Base>
{
	public:
		using base_class = Base;
		using derived_class = Base;

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
		FactoryWrapper(const FactoryWrapper& p) noexcept : FactoryWrapperPtr<Base, Base>(), base(p.base), type(p.type)
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
template<> struct rBases<Base> { typedef void type; };

template<typename F>
struct FindCastablePointer {
	using base_class = typename rBases<F>::type;
	typedef typename std::conditional<!Utils::is_static_castable<base_class*, F*>::value, F, typename FindCastablePointer<base_class>::type>::type type;
	using next_base = typename rBases<type>::type;
};
template<>
struct FindCastablePointer<Base> {
	typedef Base type;
	using next_base = void;
};

template<typename F>
using CastablePointer = typename FindCastablePointer<F>::type;

template<typename T>
constexpr unsigned int PointerCount(unsigned int count = 1) { return !std::is_same<typename FindCastablePointer<T>::type, Base>::value ? PointerCount<typename FindCastablePointer<T>::next_base>(count + 1) : count; }

template<>
constexpr unsigned int PointerCount<void>(unsigned int) { return 0; }

template<typename D>
class FactoryWrapper : protected FactoryWrapperPtr<typename rBases<D>::type, D>, public FactoryWrapper<typename rBases<D>::type>
{
	public:
		using base_class = typename rBases<D>::type;
		using derived_class = D;

		friend class GameFactory;

		template<typename T, typename U>
		friend Expected<FactoryWrapper<T>> vaultcast(const FactoryWrapper<U>& object) noexcept;
		template<typename T, typename U>
		friend Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>&& object) noexcept;

	protected:
		FactoryWrapper(derived_class* ptr, unsigned int type) noexcept
		    : FactoryWrapperPtr<base_class, derived_class>(ptr),
		      FactoryWrapper<base_class>(ptr, type) {}
		FactoryWrapper(Base* base, unsigned int type, derived_class* casted = nullptr) noexcept
		    : FactoryWrapperPtr<base_class, derived_class>(FactoryWrapper<Base>::validate<derived_class>(type) ? (casted = Utils::static_or_dynamic_cast<derived_class>(base)) : (casted = nullptr)),
		      FactoryWrapper<base_class>(casted, type) {}
		template<typename T> FactoryWrapper(const FactoryWrapper<T>& p) noexcept : FactoryWrapper(p.operator->(), p.GetType()) {}
		template<typename T> FactoryWrapper(FactoryWrapper<T>&& p) noexcept : FactoryWrapper(p.operator->(), p.GetType()) { GameFactory::Free(p); }

	public:
		FactoryWrapper() = default;
		FactoryWrapper(const FactoryWrapper&) = default;
		FactoryWrapper& operator=(const FactoryWrapper&) = default;
		FactoryWrapper(FactoryWrapper&&) = default;
		FactoryWrapper& operator=(FactoryWrapper&&) = default;
		~FactoryWrapper() = default;

		derived_class* operator->() const noexcept {
			using castable_pointer_wrapper = FactoryWrapper<CastablePointer<derived_class>>;
			return static_cast<derived_class*>(FactoryWrapperPtr<typename castable_pointer_wrapper::base_class, typename castable_pointer_wrapper::derived_class>::get_ptr(FactoryWrapper<Base>::base));
		}
		derived_class& operator*() const noexcept { return *operator->(); }
		operator derived_class*() const noexcept { return operator->(); }
};

#define GF_TYPE_WRAPPER(derived_class, base_class, identity, token)                                \
template<> struct rBases<derived_class> { typedef base_class type; };                              \
template<> struct rTypes<derived_class> { enum { value = identity }; };                            \
template<> struct rTypesToken<derived_class> { enum { value = token }; };                          \
typedef std::vector<derived_class*> derived_class##s;                                              \
typedef FactoryWrapper<derived_class> Factory##derived_class;                                      \
typedef std::vector<FactoryWrapper<derived_class>> Factory##derived_class##s;                      \
typedef Expected<FactoryWrapper<derived_class>> Expected##derived_class;                           \
typedef std::vector<Expected<FactoryWrapper<derived_class>>> Expected##derived_class##s;           \
static_assert(sizeof(Factory##derived_class) == (PointerCount<derived_class>() * sizeof(void*)) + sizeof(unsigned int), "Unexpected object size");

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
