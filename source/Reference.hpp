#ifndef REFERENCE_H
#define REFERENCE_H

#include "vaultmp.hpp"
#include "Value.hpp"
#include "RakNet.hpp"
#include "Base.hpp"
#include "Interface.hpp"
#include "ReferenceTypes.hpp"
#include "GameFactory.hpp"
#include "packet/PacketFactory.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <queue>

/**
 * \brief The base class for all in-game types
 *
 * Data specific to References are a reference ID and a base ID
 */

class Reference : public virtual Base
{
		friend class GameFactory;

	private:
		typedef std::unordered_map<unsigned int, RakNet::NetworkID> RefIDs;

#ifdef VAULTMP_DEBUG
		static DebugInput<Reference> debug;
#endif

		static Guarded<RefIDs> refIDs;

#ifndef VAULTSERVER
		std::queue<std::function<void()>> tasks;
#endif

		Value<unsigned int> refID;
		Value<unsigned int> baseID;

		template<typename T>
		struct PickBy_ {
			static RakNet::NetworkID PickBy(T id) noexcept;
			template<template<typename...> class C> static std::vector<RakNet::NetworkID> PickBy(C<T>&& ids) noexcept;
			template<template<typename...> class C> static std::vector<RakNet::NetworkID> PickBy(const C<T>& ids) noexcept;
		};

		template<typename T> inline static RakNet::NetworkID PickBy(T id) noexcept { return PickBy_<T>::PickBy(id); }
		template<typename T, template<typename...> class C> inline static std::vector<RakNet::NetworkID> PickBy(C<T>&& ids) noexcept { return PickBy_<T>::PickBy(std::move(ids)); }
		template<typename T, template<typename...> class C> inline static std::vector<RakNet::NetworkID> PickBy(const C<T>& ids) noexcept { return PickBy_<T>::PickBy(ids); }

		Reference(const Reference&) = delete;
		Reference& operator=(const Reference&) = delete;

	protected:
		//static unsigned int ResolveIndex(unsigned int baseID);

		template<typename T> static Lockable* SetObjectValue(Value<T>& dest, const T& value);

		Reference(unsigned int refID, unsigned int baseID);
		Reference(unsigned int baseID) : Reference(0x00000000, baseID) {}
		Reference(const pPacket& packet);
		Reference(pPacket&& packet) : Reference(packet) {};

	public:
		virtual ~Reference() noexcept;

		/**
		 * \brief Retrieves the Reference's reference ID
		 */
		unsigned int GetReference() const;
		/**
		 * \brief Retrieves the Reference's base ID
		 */
		unsigned int GetBase() const;

		/**
		 * \brief Sets the References's reference ID
		 */
		Lockable* SetReference(unsigned int refID);
		/**
		 * \brief Sets the References's base ID
		 */
#ifdef VAULTSERVER
		virtual Lockable* SetBase(unsigned int baseID);
#else
		Lockable* SetBase(unsigned int baseID);
#endif

		/**
		 * \brief Determines if the reference ID is persistent
		 */
		bool IsPersistent() const;
		/**
		 * \brief Returns a constant Parameter used to pass the reference ID of this Reference to the Interface
		 */
		RawParameter GetReferenceParam() const { return RawParameter(refID.get()); };
		/**
		 * \brief Returns a constant Parameter used to pass the base ID of this Reference to the Interface
		 */
		RawParameter GetBaseParam() const { return RawParameter(baseID.get()); };

#ifndef VAULTSERVER
		/**
		 * \brief Enqueues a task
		 */
		void Enqueue(const std::function<void()>& task);
		/**
		 * \brief Executes all tasks
		 */
		void Work();
		/**
		 * \brief Releases all tasks
		 */
		void Release();
#endif

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

template<typename T>
Lockable* Reference::SetObjectValue(Value<T>& dest, const T& value)
{
	if (dest.get() == value)
		return nullptr;

	if (!dest.set(value))
		return nullptr;

	return &dest;
}

template<> Lockable* Reference::SetObjectValue(Value<float>& dest, const float& value);
template<> Lockable* Reference::SetObjectValue(Value<std::tuple<float, float, float>>& dest, const std::tuple<float, float, float>& value);

template<>
struct Reference::PickBy_<unsigned int> {
	static RakNet::NetworkID PickBy(unsigned int id) noexcept {
		return refIDs.Operate([id](RefIDs& refIDs) {
			return refIDs[id];
		});
	}

	template<template<typename...> class C>
	static std::vector<RakNet::NetworkID> PickBy(C<unsigned int>&& ids) noexcept {
		return PickBy(ids);
	}

	template<template<typename...> class C>
	static std::vector<RakNet::NetworkID> PickBy(const C<unsigned int>& ids) noexcept {
		return refIDs.Operate([&ids](RefIDs& refIDs) {
			std::vector<RakNet::NetworkID> result;

			for (auto id : ids)
				result.emplace_back(refIDs[id]);

			return result;
		});
	}
};

class ReferenceFunctor : public VaultFunctor
{
	private:
		unsigned int _flags;
		RakNet::NetworkID id;

	protected:
		ReferenceFunctor(unsigned int flags, RakNet::NetworkID id) : VaultFunctor(), _flags(flags), id(id) {}
		virtual ~ReferenceFunctor() {}

		virtual bool filter(FactoryWrapper<Reference>& reference) = 0;

		unsigned int flags() { return _flags; }
		RakNet::NetworkID get() { return id; }
};

GF_TYPE_WRAPPER(Reference, Base, ID_REFERENCE, ALL_REFERENCES)

PF_MAKE(ID_REFERENCE_NEW, pGeneratorReferenceExtend, unsigned int, unsigned int)
template<>
inline const typename pTypesMap<pTypes::ID_REFERENCE_NEW>::type* PacketFactory::Cast_<pTypes::ID_REFERENCE_NEW>::Cast(const pPacket* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_REFERENCE_NEW ||
		type == pTypes::ID_OBJECT_NEW ||
		type == pTypes::ID_ITEM_NEW ||
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_REFERENCE_NEW>::type*>(packet) : nullptr;
}

#endif
