#ifndef PACKETFACTORY_H
#define PACKETFACTORY_H

#include "vaultmp.h"
#include "VaultException.h"
#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>
#include <tuple>
#include <cstring>
#include <memory>

typedef unsigned char pTypesSize;

enum class pTypes : pTypesSize
{
	ID_GAME_AUTH = ID_GAME_FIRST,
	ID_GAME_LOAD,
	ID_GAME_MOD,
	ID_GAME_START,
	ID_GAME_END,
	ID_GAME_MESSAGE,
	ID_GAME_CHAT,
	ID_GAME_GLOBAL,
	ID_GAME_WEATHER,
	ID_GAME_BASE,
	ID_GAME_DELETED,

	ID_OBJECT_NEW,
	ID_ITEM_NEW,
	ID_CONTAINER_NEW,
	ID_ACTOR_NEW,
	ID_PLAYER_NEW,
	ID_OBJECT_REMOVE,

	ID_WINDOW_NEW,
	ID_BUTTON_NEW,
	ID_TEXT_NEW,
	ID_EDIT_NEW,
	ID_WINDOW_REMOVE,

	ID_UPDATE_NAME,
	ID_UPDATE_POS,
	ID_UPDATE_ANGLE,
	ID_UPDATE_CELL,
	ID_UPDATE_LOCK,
	ID_UPDATE_OWNER,
	ID_UPDATE_ACTIVATE,
	ID_UPDATE_COUNT,
	ID_UPDATE_CONDITION,
	ID_UPDATE_EQUIPPED,
	ID_UPDATE_VALUE,
	ID_UPDATE_STATE,
	ID_UPDATE_RACE,
	ID_UPDATE_SEX,
	ID_UPDATE_DEAD,
	ID_UPDATE_FIREWEAPON,
	ID_UPDATE_IDLE,
	ID_UPDATE_CONTROL,
	ID_UPDATE_INTERIOR,
	ID_UPDATE_EXTERIOR,
	ID_UPDATE_CONTEXT,
	ID_UPDATE_CONSOLE,
	ID_UPDATE_WPOS,
	ID_UPDATE_WSIZE,
	ID_UPDATE_WVISIBLE,
	ID_UPDATE_WLOCKED,
	ID_UPDATE_WTEXT,
	ID_UPDATE_WMAXLEN,
	ID_UPDATE_WVALID,
	ID_UPDATE_WCLICK,
	ID_UPDATE_WMODE
};

template<pTypes>
struct pTypesMap;

class pDefault;
typedef std::unique_ptr<pDefault> pPacket;

class PacketFactory
{
	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<PacketFactory> debug;
#endif

		PacketFactory() = delete;

		template<pTypes type, typename... Args>
		struct Create_ {
			inline static pPacket Create(Args&&... args) {
				return pPacket(new typename pTypesMap<type>::type(std::forward<Args>(args)...));
			}
		};

		template<pTypes type>
		struct Cast_ {
			inline static const typename pTypesMap<type>::type* Cast(const pDefault* packet);
		};

		template<pTypes type, typename... Args>
		struct Access_ {
			inline static void Access(const pDefault* packet, Args&... args) {
				Cast<type>(packet)->access(std::forward<Args&>(args)...);
			}
		};

	public:
		template<pTypes type, typename... Args>
		inline static pPacket Create(Args&&... args) { return Create_<type, Args...>::Create(std::forward<Args>(args)...); }

		template<pTypes type>
		inline static const typename pTypesMap<type>::type* Cast(const pDefault* packet) { return Cast_<type>::Cast(packet); }

		template<pTypes type>
		inline static const typename pTypesMap<type>::type* Cast(const pPacket& packet) { return Cast_<type>::Cast(packet.get()); }

		template<pTypes type, typename... Args>
		inline static void Access(const pDefault* packet, Args&... args) { Access_<type, Args...>::Access(packet, std::forward<Args&>(args)...); }

		template<pTypes type, typename... Args>
		inline static void Access(const pPacket& packet, Args&... args) { Access_<type, Args...>::Access(packet.get(), std::forward<Args&>(args)...); }

		template<typename T>
		inline static T Pop(const pDefault* packet);

		static pPacket Init(const unsigned char* stream, unsigned int len);
};

class pDefault
{
		friend class PacketFactory;

	private:
		pDefault(const pDefault&) = delete;
		pDefault& operator=(const pDefault&) = delete;

		template<size_t>
		struct tuple_count {};

		template<typename... T, size_t N>
		void unpack_tuple(const std::tuple<T...>&, tuple_count<N>);
		template<typename... T>
		void unpack_tuple(const std::tuple<T...>&, tuple_count<0>);
		template<typename T, size_t N, size_t I>
		void unpack_array(const std::array<T, N>&, tuple_count<I>);
		template<typename T, size_t N>
		void unpack_array(const std::array<T, N>&, tuple_count<0>);

		template<typename... T, size_t N>
		void pack_tuple(std::tuple<T...>&, tuple_count<N>) const;
		template<typename... T>
		void pack_tuple(std::tuple<T...>&, tuple_count<0>) const;
		template<typename T, size_t N, size_t I>
		void pack_array(std::array<T, N>&, tuple_count<I>) const;
		template<typename T, size_t N>
		void pack_array(std::array<T, N>&, tuple_count<0>) const;

		std::vector<unsigned char> data;
		mutable unsigned int location;

	protected:
		pDefault(pTypes type) : location(sizeof(pTypes))
		{
			construct(type);
		}

		pDefault(const unsigned char* stream, unsigned int len) : data(stream, stream + len), location(sizeof(pTypes))
		{

		}

		template<typename T, typename... Args>
		void construct(const T&, const Args&...);
		void construct() {}

		template<typename... Args>
		void construct(const pPacket&, const Args&...);

		template<typename... Args>
		void construct(const std::string&, const Args&...);

		template<typename T, typename... Args>
		void construct(const std::vector<T>&, const Args&...);

		template<typename T, typename... Args>
		void construct(const std::list<T>&, const Args&...);

		template<typename K, typename V, typename... Args>
		void construct(const std::map<K, V>&, const Args&...);

		template<typename K, typename V, typename... Args>
		void construct(const std::unordered_map<K, V>&, const Args&...);

		template<typename T1, typename T2, typename... Args>
		void construct(const std::pair<T1, T2>&, const Args&...);

		template<typename... T, typename... Args>
		void construct(const std::tuple<T...>&, const Args&...);

		template<typename T, size_t N, typename... Args>
		void construct(const std::array<T, N>&, const Args&...);

		template<typename T, typename... Args>
		void deconstruct(T&, Args&...) const;
		template<typename T>
		T deconstruct_single() const;
		void deconstruct() const {}

		template<typename... Args>
		void deconstruct(std::string&, Args&...) const;

		template<typename T, typename... Args>
		void deconstruct(std::vector<T>&, Args&...) const;

		template<typename T, typename... Args>
		void deconstruct(std::list<T>&, Args&...) const;

		template<typename K, typename V, typename... Args>
		void deconstruct(std::map<K, V>&, Args&...) const;

		template<typename K, typename V, typename... Args>
		void deconstruct(std::unordered_map<K, V>&, Args&...) const;

		template<typename T1, typename T2, typename... Args>
		void deconstruct(std::pair<T1, T2>&, Args&...) const;

		template<typename... T, typename... Args>
		void deconstruct(std::tuple<T...>&, Args&...) const;

		template<typename T, size_t N, typename... Args>
		void deconstruct(std::array<T, N>&, Args&...) const;

	public:
		virtual ~pDefault() = default;

		const unsigned char* get() const
		{
			return &data[0];
		}

		pTypes type() const
		{
			return static_cast<pTypes>(data[0]);
		}

		unsigned int length() const
		{
			return data.size();
		}
};

template<pTypes type>
inline const typename pTypesMap<type>::type* PacketFactory::Cast_<type>::Cast(const pDefault* packet) {
	return packet->type() == type ? static_cast<const typename pTypesMap<type>::type*>(packet) : nullptr;
}

template<typename T, typename... Args>
void pDefault::construct(const T& arg, const Args&... args)
{
	// is_trivially_copyable not implemented in GCC as of now
	static_assert(std::is_trivial<T>::value, "Type cannot be trivially copied");

	data.insert(data.end(), reinterpret_cast<const unsigned char*>(&arg), reinterpret_cast<const unsigned char*>(&arg) + sizeof(T));

	construct(std::forward<const Args&>(args)...);
}

template<typename... Args>
void pDefault::construct(const pPacket& arg, const Args&... args)
{
	const unsigned char* _data = arg->get();
	unsigned int length = arg->length();

	construct(length);
	data.insert(data.end(), _data, _data + length);

	construct(std::forward<const Args&>(args)...);
}

template<typename... Args>
void pDefault::construct(const std::string& arg, const Args&...args)
{
	size_t length = arg.length();
	const unsigned char* str = reinterpret_cast<const unsigned char*>(arg.c_str());

	data.insert(data.end(), str, str + length + 1);

	construct(std::forward<const Args&>(args)...);
}

template<typename T, typename... Args>
void pDefault::construct(const std::vector<T>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(std::forward<const Args&>(args)...);
}

template<typename T, typename... Args>
void pDefault::construct(const std::list<T>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(std::forward<const Args&>(args)...);
}

template<typename K, typename V, typename... Args>
void pDefault::construct(const std::map<K, V>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(std::forward<const Args&>(args)...);
}

template<typename K, typename V, typename... Args>
void pDefault::construct(const std::unordered_map<K, V>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(std::forward<const Args&>(args)...);
}

template<typename T1, typename T2, typename... Args>
void pDefault::construct(const std::pair<T1, T2>& arg, const Args&...args)
{
	construct(arg.first);
	construct(arg.second);
	construct(std::forward<const Args&>(args)...);
}

template<typename... T, typename... Args>
void pDefault::construct(const std::tuple<T...>& arg, const Args&...args)
{
	unpack_tuple(arg, tuple_count<sizeof...(T) - 1>());
	construct(std::forward<const Args&>(args)...);
}

template<typename T, size_t N, typename... Args>
void pDefault::construct(const std::array<T, N>& arg, const Args&...args)
{
	unpack_array(arg, tuple_count<N - 1>());
	construct(std::forward<const Args&>(args)...);
}

template<typename... T, size_t N>
void pDefault::unpack_tuple(const std::tuple<T...>& arg, tuple_count<N>)
{
	construct(std::get<N>(arg));
	unpack_tuple(arg, tuple_count<N - 1>());
}

template<typename... T>
void pDefault::unpack_tuple(const std::tuple<T...>& arg, tuple_count<0>)
{
	construct(std::get<0>(arg));
}

template<typename T, size_t N, size_t I>
void pDefault::unpack_array(const std::array<T, N>& arg, tuple_count<I>)
{
	construct(arg[I]);
	unpack_array(arg, tuple_count<I - 1>());
}

template<typename T, size_t N>
void pDefault::unpack_array(const std::array<T, N>& arg, tuple_count<0>)
{
	construct(arg[0]);
}

template<typename T, typename... Args>
void pDefault::deconstruct(T& arg, Args&... args) const
{
	arg = deconstruct_single<T>();
	deconstruct(std::forward<Args&>(args)...);
}

template<typename T>
T pDefault::deconstruct_single() const
{
	// is_trivially_copyable not implemented in GCC as of now
	static_assert(std::is_trivial<T>::value, "Type cannot be trivially copied");

	if (location + sizeof(T) > this->length())
		throw VaultException("Reading past the end of packet").stacktrace();

	location += sizeof(T);

	return *reinterpret_cast<const T*>(&data[location - sizeof(T)]);
}

template<>
inline pPacket pDefault::deconstruct_single() const
{
	unsigned int length = deconstruct_single<unsigned int>();

	if (location + length > this->length())
		throw VaultException("Reading past the end of packet").stacktrace();

	pPacket packet = PacketFactory::Init(&data[location], length);

	location += length;

	return packet;
}

template<typename... Args>
void pDefault::deconstruct(std::string& arg, Args&... args) const
{
	size_t length = std::strlen(reinterpret_cast<const char*>(&data[location]));

	if (location + length + 1 > this->length())
		throw VaultException("Reading past the end of packet").stacktrace();

	arg.assign(reinterpret_cast<const char*>(&data[location]), length);

	location += length + 1;

	deconstruct(std::forward<Args&>(args)...);
}

template<typename T, typename... Args>
void pDefault::deconstruct(std::vector<T>& arg, Args&... args) const
{
	size_t size = deconstruct_single<size_t>();

	arg.resize(size);

	for (auto& element : arg)
		deconstruct(element);

	deconstruct(std::forward<Args&>(args)...);
}

template<typename T, typename... Args>
void pDefault::deconstruct(std::list<T>& arg, Args&... args) const
{
	size_t size = deconstruct_single<size_t>();

	arg.resize(size);

	for (auto& element : arg)
		deconstruct(element);

	deconstruct(std::forward<Args&>(args)...);
}

template<typename K, typename V, typename... Args>
void pDefault::deconstruct(std::map<K, V>& arg, Args&... args) const
{
	size_t size = deconstruct_single<size_t>();

	arg.clear();

	for (size_t i = 0; i < size; ++i)
	{
		std::pair<K, V> data;
		deconstruct(data);
		// arg.emplace_hint(arg.end(), move(data));
		arg.insert(std::move(data));
	}

	deconstruct(std::forward<Args&>(args)...);
}

template<typename K, typename V, typename... Args>
void pDefault::deconstruct(std::unordered_map<K, V>& arg, Args&... args) const
{
	size_t size = deconstruct_single<size_t>();

	arg.clear();

	for (size_t i = 0; i < size; ++i)
	{
		std::pair<K, V> data;
		deconstruct(data);
		arg.emplace(std::move(data));
	}

	deconstruct(std::forward<Args&>(args)...);
}

template<typename T1, typename T2, typename... Args>
void pDefault::deconstruct(std::pair<T1, T2>& arg, Args&... args) const
{
	deconstruct(arg.first);
	deconstruct(arg.second);
	deconstruct(std::forward<Args&>(args)...);
}

template<typename... T, typename... Args>
void pDefault::deconstruct(std::tuple<T...>& arg, Args&... args) const
{
	pack_tuple(arg, tuple_count<sizeof...(T) - 1>());
	deconstruct(std::forward<Args&>(args)...);
}

template<typename T, size_t N, typename... Args>
void pDefault::deconstruct(std::array<T, N>& arg, Args&... args) const
{
	pack_array(arg, tuple_count<N - 1>());
	deconstruct(std::forward<Args&>(args)...);
}

template<typename... T, size_t N>
void pDefault::pack_tuple(std::tuple<T...>& arg, tuple_count<N>) const
{
	deconstruct(std::get<N>(arg));
	pack_tuple(arg, tuple_count<N - 1>());
}

template<typename... T>
void pDefault::pack_tuple(std::tuple<T...>& arg, tuple_count<0>) const
{
	deconstruct(std::get<0>(arg));
}

template<typename T, size_t N, size_t I>
void pDefault::pack_array(std::array<T, N>& arg, tuple_count<I>) const
{
	deconstruct(arg[I]);
	pack_array(arg, tuple_count<I - 1>());
}

template<typename T, size_t N>
void pDefault::pack_array(std::array<T, N>& arg, tuple_count<0>) const
{
	deconstruct(arg[0]);
}

template<typename T>
inline T PacketFactory::Pop(const pDefault* packet) {
	return packet->deconstruct_single<T>();
}

class pReferenceDefault : public pDefault
{
		friend class PacketFactory;

	protected:
		pReferenceDefault(pTypes type) : pDefault(type)
		{

		}

		pReferenceDefault(pTypes type, RakNet::NetworkID id) : pDefault(type)
		{
			construct(id);
		}

		pReferenceDefault(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}
};

class pReferenceNewDefault : public pReferenceDefault
{
		friend class PacketFactory;

	protected:
		pReferenceNewDefault(pTypes type) : pReferenceDefault(type)
		{

		}

		pReferenceNewDefault(pTypes type, RakNet::NetworkID id, unsigned int refID, unsigned int baseID) : pReferenceDefault(type, id)
		{
			construct(refID, baseID);
		}

		pReferenceNewDefault(const unsigned char* stream, unsigned int len) : pReferenceDefault(stream, len)
		{

		}
};

template<pTypes Type, typename... Args>
class pGeneratorDefault : public pDefault
{
		friend class PacketFactory;

	private:
		pGeneratorDefault(const Args&... args) : pDefault(Type)
		{
			construct(std::forward<const Args&>(args)...);
		}
		pGeneratorDefault(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(Args&... args) const
		{
			deconstruct(std::forward<Args&>(args)...);
		}
};

template<pTypes Type, typename... Args>
class pGeneratorReference : public pReferenceDefault
{
		friend class PacketFactory;

	private:
		pGeneratorReference(const RakNet::NetworkID& id, const Args&... args) : pReferenceDefault(Type, id)
		{
			construct(std::forward<const Args&>(args)...);
		}
		pGeneratorReference(const unsigned char* stream, unsigned int len) : pReferenceDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, Args&... args) const
		{
			deconstruct(id, std::forward<Args&>(args)...);
		}
};

template<pTypes Type, typename... Args>
class pGeneratorReferenceNew : public pReferenceNewDefault
{
		friend class PacketFactory;

	private:
		pGeneratorReferenceNew(const RakNet::NetworkID& id, unsigned int refID, unsigned int baseID, const Args&... args) : pReferenceNewDefault(Type, id, refID, baseID)
		{
			construct(std::forward<const Args&>(args)...);
		}
		pGeneratorReferenceNew(const unsigned char* stream, unsigned int len) : pReferenceNewDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& refID, unsigned int& baseID, Args&... args) const
		{
			deconstruct(id, refID, baseID, std::forward<Args&>(args)...);
		}
};

template<pTypes Type, typename... Args>
class pGeneratorReferenceExtend : public pReferenceNewDefault
{
		friend class PacketFactory;

	private:
		pGeneratorReferenceExtend(const pPacket& sub, const Args&... args) : pReferenceNewDefault(Type)
		{
			construct(sub, std::forward<const Args&>(args)...);
		}
		pGeneratorReferenceExtend(const unsigned char* stream, unsigned int len) : pReferenceNewDefault(stream, len)
		{

		}

		void access(Args&... args) const
		{
			deconstruct(std::forward<Args&>(args)...);
		}
};

template<> struct pTypesMap<pTypes::ID_GAME_AUTH> { typedef pGeneratorDefault<pTypes::ID_GAME_AUTH, std::string, std::string> type; };
template<> struct pTypesMap<pTypes::ID_GAME_LOAD> { typedef pGeneratorDefault<pTypes::ID_GAME_LOAD> type; };
template<> struct pTypesMap<pTypes::ID_GAME_MOD> { typedef pGeneratorDefault<pTypes::ID_GAME_MOD, std::string, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_GAME_START> { typedef pGeneratorDefault<pTypes::ID_GAME_START> type; };
template<> struct pTypesMap<pTypes::ID_GAME_END> { typedef pGeneratorDefault<pTypes::ID_GAME_END, Reason> type; };
template<> struct pTypesMap<pTypes::ID_GAME_MESSAGE> { typedef pGeneratorDefault<pTypes::ID_GAME_MESSAGE, std::string, unsigned char> type; };
template<> struct pTypesMap<pTypes::ID_GAME_CHAT> { typedef pGeneratorDefault<pTypes::ID_GAME_CHAT, std::string> type; };
template<> struct pTypesMap<pTypes::ID_GAME_GLOBAL> { typedef pGeneratorDefault<pTypes::ID_GAME_GLOBAL, unsigned int, signed int> type; };
template<> struct pTypesMap<pTypes::ID_GAME_WEATHER> { typedef pGeneratorDefault<pTypes::ID_GAME_WEATHER, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_GAME_BASE> { typedef pGeneratorDefault<pTypes::ID_GAME_BASE, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_GAME_DELETED> { typedef pGeneratorDefault<pTypes::ID_GAME_DELETED, std::unordered_map<unsigned int, std::vector<unsigned int>>> type; };
template<> struct pTypesMap<pTypes::ID_OBJECT_NEW> { typedef pGeneratorReferenceNew<pTypes::ID_OBJECT_NEW, bool, std::string, double, double, double, double, double, double, unsigned int, bool, unsigned int, unsigned int> type; };
template<>
inline const typename pTypesMap<pTypes::ID_OBJECT_NEW>::type* PacketFactory::Cast_<pTypes::ID_OBJECT_NEW>::Cast(const pDefault* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_OBJECT_NEW ||
		type == pTypes::ID_ITEM_NEW ||
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_OBJECT_NEW>::type*>(packet) : nullptr;
}
template<> struct pTypesMap<pTypes::ID_ITEM_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_ITEM_NEW, RakNet::NetworkID, unsigned int, double, bool, bool, bool> type; };
template<> struct pTypesMap<pTypes::ID_CONTAINER_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_CONTAINER_NEW, std::vector<pPacket>> type; };
template<>
inline const typename pTypesMap<pTypes::ID_CONTAINER_NEW>::type* PacketFactory::Cast_<pTypes::ID_CONTAINER_NEW>::Cast(const pDefault* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_CONTAINER_NEW>::type*>(packet) : nullptr;
}
template<> struct pTypesMap<pTypes::ID_ACTOR_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_ACTOR_NEW, std::map<unsigned char, double>, std::map<unsigned char, double>, unsigned int, signed int, unsigned int, unsigned char, unsigned char, unsigned char, bool, bool, bool, bool> type; };
template<>
inline const typename pTypesMap<pTypes::ID_ACTOR_NEW>::type* PacketFactory::Cast_<pTypes::ID_ACTOR_NEW>::Cast(const pDefault* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_ACTOR_NEW>::type*>(packet) : nullptr;
}
template<> struct pTypesMap<pTypes::ID_PLAYER_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_PLAYER_NEW, std::map<unsigned char, std::pair<unsigned char, bool>>> type; };
template<> struct pTypesMap<pTypes::ID_OBJECT_REMOVE> { typedef pGeneratorReference<pTypes::ID_OBJECT_REMOVE, bool> type; };
template<> struct pTypesMap<pTypes::ID_WINDOW_NEW> { typedef pGeneratorReference<pTypes::ID_WINDOW_NEW, RakNet::NetworkID, std::string, std::tuple<double, double, double, double>, std::tuple<double, double, double, double>, bool, bool, std::string> type; };
template<>
inline const typename pTypesMap<pTypes::ID_WINDOW_NEW>::type* PacketFactory::Cast_<pTypes::ID_WINDOW_NEW>::Cast(const pDefault* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_WINDOW_NEW ||
		type == pTypes::ID_BUTTON_NEW ||
		type == pTypes::ID_TEXT_NEW ||
		type == pTypes::ID_EDIT_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_WINDOW_NEW>::type*>(packet) : nullptr;
}
template<> struct pTypesMap<pTypes::ID_BUTTON_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_BUTTON_NEW> type; };
template<> struct pTypesMap<pTypes::ID_TEXT_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_TEXT_NEW> type; };
template<> struct pTypesMap<pTypes::ID_EDIT_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_EDIT_NEW, unsigned int, std::string> type; };
template<> struct pTypesMap<pTypes::ID_WINDOW_REMOVE> { typedef pGeneratorReference<pTypes::ID_WINDOW_REMOVE> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_NAME> { typedef pGeneratorReference<pTypes::ID_UPDATE_NAME, std::string> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_POS> { typedef pGeneratorReference<pTypes::ID_UPDATE_POS, double, double, double> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_ANGLE> { typedef pGeneratorReference<pTypes::ID_UPDATE_ANGLE, unsigned char, double> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_CELL> { typedef pGeneratorReference<pTypes::ID_UPDATE_CELL, unsigned int, double, double, double> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_LOCK> { typedef pGeneratorReference<pTypes::ID_UPDATE_LOCK, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_OWNER> { typedef pGeneratorReference<pTypes::ID_UPDATE_OWNER, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_ACTIVATE> { typedef pGeneratorReference<pTypes::ID_UPDATE_ACTIVATE, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_COUNT> { typedef pGeneratorReference<pTypes::ID_UPDATE_COUNT, unsigned int, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_CONDITION> { typedef pGeneratorReference<pTypes::ID_UPDATE_CONDITION, double, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_EQUIPPED> { typedef pGeneratorReference<pTypes::ID_UPDATE_EQUIPPED, bool, bool, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_VALUE> { typedef pGeneratorReference<pTypes::ID_UPDATE_VALUE, bool, unsigned char, double> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_STATE> { typedef pGeneratorReference<pTypes::ID_UPDATE_STATE, unsigned int, unsigned char, unsigned char, unsigned char, bool, bool, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_RACE> { typedef pGeneratorReference<pTypes::ID_UPDATE_RACE, unsigned int, signed int, signed int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_SEX> { typedef pGeneratorReference<pTypes::ID_UPDATE_SEX, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_DEAD> { typedef pGeneratorReference<pTypes::ID_UPDATE_DEAD, bool, unsigned short, signed char> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_FIREWEAPON> { typedef pGeneratorReference<pTypes::ID_UPDATE_FIREWEAPON, unsigned int, double> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_IDLE> { typedef pGeneratorReference<pTypes::ID_UPDATE_IDLE, unsigned int, std::string> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_CONTROL> { typedef pGeneratorReference<pTypes::ID_UPDATE_CONTROL, unsigned char, unsigned char> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_INTERIOR> { typedef pGeneratorReference<pTypes::ID_UPDATE_INTERIOR, std::string, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_EXTERIOR> { typedef pGeneratorReference<pTypes::ID_UPDATE_EXTERIOR, unsigned int, signed int, signed int, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_CONTEXT> { typedef pGeneratorReference<pTypes::ID_UPDATE_CONTEXT, std::array<unsigned int, 9>, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_CONSOLE> { typedef pGeneratorReference<pTypes::ID_UPDATE_CONSOLE, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WPOS> { typedef pGeneratorReference<pTypes::ID_UPDATE_WPOS, std::tuple<double, double, double, double>> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WSIZE> { typedef pGeneratorReference<pTypes::ID_UPDATE_WSIZE, std::tuple<double, double, double, double>> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WLOCKED> { typedef pGeneratorReference<pTypes::ID_UPDATE_WLOCKED, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WVISIBLE> { typedef pGeneratorReference<pTypes::ID_UPDATE_WVISIBLE, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WTEXT> { typedef pGeneratorReference<pTypes::ID_UPDATE_WTEXT, std::string> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WMAXLEN> { typedef pGeneratorReference<pTypes::ID_UPDATE_WMAXLEN, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WVALID> { typedef pGeneratorReference<pTypes::ID_UPDATE_WVALID, std::string> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WCLICK> { typedef pGeneratorReference<pTypes::ID_UPDATE_WCLICK> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WMODE> { typedef pGeneratorDefault<pTypes::ID_UPDATE_WMODE, bool> type; };

#endif
