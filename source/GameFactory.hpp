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
	static typename OperateReturn<FailPolicy::Return, OP, LaunchPolicy::Blocking, T, F, I>::type Operate(I&& id, F function) noexcept
	{
		auto base = GameFactory::Get<T>(std::forward<I>(id));

		try
		{
			auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
			return function(param);
		}
		catch (...) { return typename ObjectPolicyReturn<OP, T, F, InputPolicyHelper<I>::value>::type(); }
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Bool, OP, LaunchPolicy::Blocking, I, F> {
	static typename OperateReturn<FailPolicy::Bool, OP, LaunchPolicy::Blocking, T, F, I>::type Operate(I&& id, F function) noexcept
	{
		static_assert(std::is_same<typename ObjectPolicyReturn<OP, T, F, InputPolicyHelper<I>::value>::type, void>::value, "Function return value disregarded");

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
	static typename OperateReturn<FailPolicy::Exception, OP, LaunchPolicy::Blocking, T, F, I>::type Operate(I&& id, F function)
	{
		auto base = GameFactory::Get<T>(std::forward<I>(id));
		auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(base);
		return function(param);
	}
};

template<typename T>
std::vector<FactoryWrapper<T>> GameFactory::GetByType(unsigned int type) noexcept
{
	std::vector<FactoryWrapper<T>> result;

	BaseList copy(cs.Operate([&result, type]() {
		result.reserve(typecount[type]);
		return instances;
	}));

	for (const auto& base : copy)
		if (base.second & type)
		{
			auto object = FactoryWrapper<T>(base.first.get(), base.second);

			if (object)
				result.emplace_back(std::move(object));
		}

	return result;
}

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
	base->virtual_initializers();
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
		friend Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>& object) noexcept;

	protected:
		Base* base;
		unsigned int type;

		FactoryWrapper(Base* base, unsigned int type) noexcept : base(static_cast<Base*>(base->StartSession())), type(type) {}

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
		friend Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>& object) noexcept;                                                                       \
                                                                                                                                                                     \
	protected:                                                                                                                                                       \
		FactoryWrapper(Base* base, unsigned int type) noexcept : FactoryWrapper<base_class>(base, type)                                                              \
		{                                                                                                                                                            \
			if (!validate<derived_class>())                                                                                                                          \
				this->base = nullptr;                                                                                                                                \
		}                                                                                                                                                            \
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
inline Expected<FactoryWrapper<T>> vaultcast_swap(FactoryWrapper<U>& object) noexcept
{
	if (!object.template validate<T>())
		return VaultException("vaultcast failed");

	auto result = FactoryWrapper<T>(object);

	GameFactory::Leave(object);

	return result;
}

template<typename T, typename U> inline Expected<FactoryWrapper<T>> vaultcast_swap(Expected<FactoryWrapper<U>>& object) noexcept { return vaultcast_swap<T>(object.get()); }
template<typename T, typename U> inline bool vaultcast_test(const FactoryWrapper<U>& object) noexcept { return object.template validate<T>(); }
template<typename T, typename U> inline bool vaultcast_test(const Expected<FactoryWrapper<U>>& object) noexcept { return const_cast<Expected<FactoryWrapper<U>>&>(object).get().template validate<T>(); }
