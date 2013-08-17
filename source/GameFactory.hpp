template<typename T>
struct GameFactory::Get_<T, RakNet::NetworkID> {
	static Expected<FactoryWrapper<T>> Get(RakNet::NetworkID id) noexcept
	{
		std::pair<ReferenceList::key_type, ReferenceList::mapped_type> reference;

		cs.Operate([id, &reference]() {
			auto it = GetShared(id);

			if (it != instances.end())
				reference = *it;
		});

		if (!reference.first)
			return VaultException("Unknown object with NetworkID %llu", id);

		return FactoryWrapper<T>(reference.first.get(), reference.second);
	}

	static std::vector<Expected<FactoryWrapper<T>>> Get(const std::vector<RakNet::NetworkID>& ids) noexcept
	{
		std::vector<Expected<FactoryWrapper<T>>> result(ids.size());
		std::multimap<ReferenceList::value_type, unsigned int> sort;

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

		for (const auto& reference : sort)
			result[reference.second] = FactoryWrapper<T>(reference.first.first.get(), reference.first.second);

		return result;
	}
};

using FailPolicy = GameFactory::FailPolicy;
using LaunchPolicy = GameFactory::LaunchPolicy;
using ObjectPolicy = GameFactory::ObjectPolicy;

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Return, OP, LaunchPolicy::Blocking, I, F, false> {
	static typename OperateReturn<FailPolicy::Return, OP, LaunchPolicy::Blocking, T, F, false>::type Operate(I&& id, F function) noexcept
	{
		auto reference = GameFactory::Get<T>(std::forward<I>(id));

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

		auto reference = GameFactory::Get<T>(std::forward<I>(id));

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
		auto reference = GameFactory::Get<T>(std::forward<I>(id));
		return function(ObjectPolicyHelper<OP, T>::Unwrap(reference));
	}
};

template<ObjectPolicy OP, typename T, typename I, typename F>
struct GameFactory::OperateFunctions<T, FailPolicy::Return, OP, LaunchPolicy::Blocking, I, F, true> {
	static typename OperateReturn<FailPolicy::Return, OP, LaunchPolicy::Blocking, T, F, true>::type Operate(std::vector<typename I::value_type>&& id, F function) noexcept
	{
		auto reference = GameFactory::Get<T>(std::forward<I>(id));

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

		auto reference = GameFactory::Get<T>(std::forward<I>(id));

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
		auto reference = GameFactory::Get<T>(std::forward<I>(id));
		auto&& param = ObjectPolicyHelper<OP, T>::Unwrap(reference); // depending on OP, returns either a reference or value
		return function(param);
	}
};

template<typename T>
std::vector<FactoryWrapper<T>> GameFactory::GetByType(unsigned int type) noexcept
{
	std::vector<FactoryWrapper<T>> result;
	ReferenceList copy;

	cs.Operate([&result, &copy, type]() {
		result.reserve(typecount[type]);
		copy = std::move(instances);
	});

	for (const auto& reference : copy)
		if (reference.second & type)
		{
			auto object = FactoryWrapper<T>(reference.first.get(), reference.second);

			if (object)
				result.emplace_back(std::move(object));
		}

	return result;
}

template<typename T>
void GameFactory::Leave(FactoryWrapper<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::Leave Reference is NULL").stacktrace();

	_reference->EndSession();
	reference.reference = nullptr;
	reference.type = 0x00;
}

template<typename T, typename... Args>
RakNet::NetworkID GameFactory::Create(Args&&... args)
{
	static_assert(std::is_base_of<Reference, T>::value, "T must be derived from Reference");

	std::shared_ptr<Reference> reference(new T(std::forward<Args>(args)...));
	constexpr unsigned int type = rTypes<T>::value;

	RakNet::NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->virtual_initializers();
#endif

	cs.Operate([id, type, &reference]() {
		++typecount[type];
		// emplace
		index[id] = instances.insert(make_pair(std::move(reference), type)).first;
	});

	return id;
}

template<typename T>
RakNet::NetworkID GameFactory::Create(const pDefault* packet)
{
	static_assert(std::is_base_of<Reference, T>::value, "T must be derived from Reference");

	std::shared_ptr<Reference> reference(new T(packet));
	constexpr unsigned int type = rTypes<T>::value;

	RakNet::NetworkID id = reference->GetNetworkID();

#ifdef VAULTSERVER
	reference->virtual_initializers();
#endif

	cs.Operate([id, type, &reference]() {
		++typecount[type];
		// emplace
		index[id] = instances.insert(make_pair(std::move(reference), type)).first;
	});

	return id;
}

template<typename T>
RakNet::NetworkID GameFactory::Destroy(FactoryWrapper<T>& reference)
{
	Reference* _reference = reference.reference;

	if (!_reference)
		throw VaultException("GameFactory::Destroy Reference is NULL").stacktrace();

	RakNet::NetworkID id = _reference->GetNetworkID();

#ifdef VAULTMP_DEBUG
	debug.print("Reference ", hex, _reference->GetReference(), " with base ",  _reference->GetBase(), " and NetworkID ", dec, _reference->GetNetworkID(), " (type: ", typeid(*_reference).name(), ") to be destructed");
#endif

	ReferenceList::key_type copy; // because the destructor of a type may also delete references, the actual destructor call must not happen within the CS block

	cs.Operate([id, _reference, &copy]() {
		auto it = GetShared(id);

		copy = it->first; // saved. will be deleted past this block
		--typecount[it->second];
		_reference->Finalize();

		instances.erase(it);
		index.erase(id);
		delrefs.emplace(id);
	});

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

	public:
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

		unsigned int GetType() const noexcept { return type; }
		Reference& operator*() const noexcept { return *reference; }
		Reference* operator->() const noexcept { return reference; }
		explicit operator bool() const noexcept { return reference; }
		bool operator==(const FactoryWrapper& p) const noexcept { return reference == p.reference; }
		bool operator!=(const FactoryWrapper& p) const noexcept { return !operator==(p); }

		template<typename T>
		inline bool validate(unsigned int type = 0x00000000) const noexcept { return type ? (type & rTypesToken<T>::value) : (this->type & rTypesToken<T>::value); }
};

#define GF_TYPE_WRAPPER(derived, base, identity, token)                                                                                                              \
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
template<> struct rTypes<derived> { enum { value = identity }; };                                                                                                    \
template<> struct rTypesToken<derived> { enum { value = token }; };                                                                                                  \
typedef FactoryWrapper<derived> Factory##derived;                                                                                                                    \
typedef std::vector<FactoryWrapper<derived>> Factory##derived##s;                                                                                                    \
typedef Expected<FactoryWrapper<derived>> Expected##derived;                                                                                                         \
typedef std::vector<Expected<FactoryWrapper<derived>>> Expected##derived##s;

#define GF_TYPE_WRAPPER_FINAL(derived, base, identity) GF_TYPE_WRAPPER(derived, base, identity, identity)

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
