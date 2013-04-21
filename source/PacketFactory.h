#ifndef PACKETFACTORY_H
#define PACKETFACTORY_H

#include <memory>
#include <type_traits>
#include <array>
#include <unordered_map>

#include "vaultmp.h"
#include "VaultException.h"
#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

enum
{
	ID_EVENT_INTERFACE_LOST,
	ID_EVENT_CLIENT_ERROR,
	ID_EVENT_SERVER_ERROR,
	ID_EVENT_GAME_STARTED,
	ID_EVENT_GAME_LOADED,
	ID_EVENT_AUTH_RECEIVED,
	ID_EVENT_CLOSE_RECEIVED,
};

enum class pTypes : unsigned char
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

	ID_UPDATE_NAME,
	ID_UPDATE_POS,
	ID_UPDATE_ANGLE,
	ID_UPDATE_CELL,
	ID_UPDATE_LOCK,
	ID_UPDATE_OWNER,
	ID_UPDATE_COUNT,
	ID_UPDATE_CONDITION,
	ID_UPDATE_CONTAINER,
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
	ID_UPDATE_CHAT,
};

enum class Reason : unsigned char
{
	ID_REASON_KICK = 0,
	ID_REASON_BAN,
	ID_REASON_ERROR,
	ID_REASON_DENIED,
	ID_REASON_QUIT,
	ID_REASON_NONE,
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

		template<pTypes type, typename... Args>
		inline static void Access(const pDefault* packet, Args&... args) { Access_<type, Args...>::Access(packet, std::forward<Args&>(args)...); }

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
		pDefault(pTypes type) : location(0), type(type)
		{
			construct(type);
		}

		pDefault(const unsigned char* stream, unsigned int len) : data(stream, stream + len), location(sizeof(pTypes))
		{

		}

		pTypes type;

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

		unsigned int length() const
		{
			return data.size();
		}
};

template<pTypes type>
inline const typename pTypesMap<type>::type* PacketFactory::Cast_<type>::Cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == type ? static_cast<const typename pTypesMap<type>::type*>(packet) : nullptr;
}

template<typename T, typename... Args>
void pDefault::construct(const T& arg, const Args&... args)
{
	// is_trivially_copyable not implemented in GCC as of now
	static_assert(std::is_trivial<T>::value, "Type cannot be trivially copied");

	data.insert(data.end(), reinterpret_cast<const unsigned char*>(&arg), reinterpret_cast<const unsigned char*>(&arg) + sizeof(T));

	construct(args...);
}

template<typename... Args>
void pDefault::construct(const pPacket& arg, const Args&... args)
{
	const unsigned char* _data = arg.get()->get();
	unsigned int length = arg.get()->length();

	construct(length);
	data.insert(data.end(), _data, _data + length);

	construct(args...);
}

template<typename... Args>
void pDefault::construct(const std::string& arg, const Args&...args)
{
	unsigned int length = arg.length();
	const unsigned char* str = reinterpret_cast<const unsigned char*>(arg.c_str());

	data.insert(data.end(), str, str + length + 1);

	construct(args...);
}

template<typename T, typename... Args>
void pDefault::construct(const std::vector<T>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(args...);
}

template<typename T, typename... Args>
void pDefault::construct(const std::list<T>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(args...);
}

template<typename K, typename V, typename... Args>
void pDefault::construct(const std::map<K, V>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(args...);
}

template<typename K, typename V, typename... Args>
void pDefault::construct(const std::unordered_map<K, V>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(args...);
}

template<typename T1, typename T2, typename... Args>
void pDefault::construct(const std::pair<T1, T2>& arg, const Args&...args)
{
	construct(arg.first);
	construct(arg.second);
	construct(args...);
}

template<typename... T, typename... Args>
void pDefault::construct(const std::tuple<T...>& arg, const Args&...args)
{
	unpack_tuple(arg, tuple_count<sizeof...(T) - 1>());
	construct(args...);
}

template<typename T, size_t N, typename... Args>
void pDefault::construct(const std::array<T, N>& arg, const Args&...args)
{
	unpack_array(arg, tuple_count<N - 1>());
	construct(args...);
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
	deconstruct(args...);
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
		throw VaultException("Reading past the end of packet");

	pPacket packet = PacketFactory::Init(&data[location], length);

	location += length;

	return packet;
}

template<typename... Args>
void pDefault::deconstruct(std::string& arg, Args&... args) const
{
	unsigned int length = strlen(reinterpret_cast<const char*>(&data[location]));

	if (location + length + 1 > this->length())
		throw VaultException("Reading past the end of packet").stacktrace();

	arg.assign(reinterpret_cast<const char*>(&data[location]), length);

	location += length + 1;

	deconstruct(args...);
}

template<typename T, typename... Args>
void pDefault::deconstruct(std::vector<T>& arg, Args&... args) const
{
	unsigned int size = deconstruct_single<unsigned int>();

	arg.clear();
	arg.resize(size);

	for (auto& element : arg)
		deconstruct(element);

	deconstruct(args...);
}

template<typename T, typename... Args>
void pDefault::deconstruct(std::list<T>& arg, Args&... args) const
{
	unsigned int size = deconstruct_single<unsigned int>();

	arg.clear();
	arg.resize(size);

	for (auto& element : arg)
		deconstruct(element);

	deconstruct(args...);
}

template<typename K, typename V, typename... Args>
void pDefault::deconstruct(std::map<K, V>& arg, Args&... args) const
{
	unsigned int size = deconstruct_single<unsigned int>();

	arg.clear();

	for (unsigned int i = 0; i < size; ++i)
	{
		std::pair<K, V> data;
		deconstruct(data);
		// arg.emplace_hint(arg.end(), move(data));
		arg.insert(std::move(data));
	}

	deconstruct(args...);
}

template<typename K, typename V, typename... Args>
void pDefault::deconstruct(std::unordered_map<K, V>& arg, Args&... args) const
{
	unsigned int size = deconstruct_single<unsigned int>();

	arg.clear();

	for (unsigned int i = 0; i < size; ++i)
	{
		std::pair<K, V> data;
		deconstruct(data);
		arg.emplace(std::move(data));
	}

	deconstruct(args...);
}

template<typename T1, typename T2, typename... Args>
void pDefault::deconstruct(std::pair<T1, T2>& arg, Args&... args) const
{
	deconstruct(arg.first);
	deconstruct(arg.second);
	deconstruct(args...);
}

template<typename... T, typename... Args>
void pDefault::deconstruct(std::tuple<T...>& arg, Args&... args) const
{
	pack_tuple(arg, tuple_count<sizeof...(T) - 1>());
	deconstruct(args...);
}

template<typename T, size_t N, typename... Args>
void pDefault::deconstruct(std::array<T, N>& arg, Args&... args) const
{
	pack_array(arg, tuple_count<N - 1>());
	deconstruct(args...);
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

class pObjectDefault : public pDefault
{
		friend class PacketFactory;

	protected:
		pObjectDefault(pTypes type) : pDefault(type)
		{

		}

		pObjectDefault(pTypes type, RakNet::NetworkID id) : pDefault(type)
		{
			construct(id);
		}

		pObjectDefault(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}
};

class pObjectNewDefault : public pObjectDefault
{
		friend class PacketFactory;

	protected:
		pObjectNewDefault(pTypes type) : pObjectDefault(type)
		{

		}

		pObjectNewDefault(pTypes type, RakNet::NetworkID id, unsigned int refID, unsigned int baseID) : pObjectDefault(type, id)
		{
			construct(refID, baseID);
		}

		pObjectNewDefault(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}
};

class pGameAuth : public pDefault
{
		friend class PacketFactory;

	private:
		pGameAuth(const std::string& name, const std::string& pwd) : pDefault(pTypes::ID_GAME_AUTH)
		{
			construct(name, pwd);
		}
		pGameAuth(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(std::string& name, std::string& pwd) const
		{
			deconstruct(name, pwd);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_AUTH> { typedef pGameAuth type; };

class pGameLoad : public pDefault
{
		friend class PacketFactory;

	private:
		pGameLoad() : pDefault(pTypes::ID_GAME_LOAD)
		{

		}
		pGameLoad(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access() const
		{

		}
};
template<> struct pTypesMap<pTypes::ID_GAME_LOAD> { typedef pGameLoad type; };

class pGameMod : public pDefault
{
		friend class PacketFactory;

	private:
		pGameMod(const std::string& modfile, unsigned int crc) : pDefault(pTypes::ID_GAME_MOD)
		{
			construct(modfile, crc);
		}
		pGameMod(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(std::string& modfile, unsigned int& crc) const
		{
			deconstruct(modfile, crc);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_MOD> { typedef pGameMod type; };

class pGameStart : public pDefault
{
		friend class PacketFactory;

	private:
		pGameStart() : pDefault(pTypes::ID_GAME_START)
		{

		}
		pGameStart(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access() const
		{

		}
};
template<> struct pTypesMap<pTypes::ID_GAME_START> { typedef pGameStart type; };

class pGameEnd : public pDefault
{
		friend class PacketFactory;

	private:
		pGameEnd(Reason reason) : pDefault(pTypes::ID_GAME_END)
		{
			construct(reason);
		}
		pGameEnd(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(Reason& reason) const
		{
			deconstruct(reason);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_END> { typedef pGameEnd type; };

class pGameMessage : public pDefault
{
		friend class PacketFactory;

	private:
		pGameMessage(const std::string& message, unsigned char emoticon) : pDefault(pTypes::ID_GAME_MESSAGE)
		{
			construct(message, emoticon);
		}
		pGameMessage(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(std::string& message, unsigned char& emoticon) const
		{
			deconstruct(message, emoticon);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_MESSAGE> { typedef pGameMessage type; };

class pGameChat : public pDefault
{
		friend class PacketFactory;

	private:
		pGameChat(const std::string& message) : pDefault(pTypes::ID_GAME_CHAT)
		{
			construct(message);
		}
		pGameChat(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(std::string& message) const
		{
			deconstruct(message);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_CHAT> { typedef pGameChat type; };

class pGameGlobal : public pDefault
{
		friend class PacketFactory;

	private:
		pGameGlobal(unsigned int global, signed int value) : pDefault(pTypes::ID_GAME_GLOBAL)
		{
			construct(global, value);
		}
		pGameGlobal(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(unsigned int& global, signed int& value) const
		{
			deconstruct(global, value);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_GLOBAL> { typedef pGameGlobal type; };

class pGameWeather : public pDefault
{
		friend class PacketFactory;

	private:
		pGameWeather(unsigned int weather) : pDefault(pTypes::ID_GAME_WEATHER)
		{
			construct(weather);
		}
		pGameWeather(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(unsigned int& weather) const
		{
			deconstruct(weather);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_WEATHER> { typedef pGameWeather type; };

class pGameBase : public pDefault
{
		friend class PacketFactory;

	private:
		pGameBase(unsigned int base) : pDefault(pTypes::ID_GAME_BASE)
		{
			construct(base);
		}
		pGameBase(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(unsigned int& base) const
		{
			deconstruct(base);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_BASE> { typedef pGameBase type; };

class pGameDeleted : public pDefault
{
		friend class PacketFactory;

	private:
		pGameDeleted(const std::unordered_map<unsigned int, std::vector<unsigned int>>& deletedStatic) : pDefault(pTypes::ID_GAME_DELETED)
		{
			construct(deletedStatic);
		}
		pGameDeleted(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(std::unordered_map<unsigned int, std::vector<unsigned int>>& deletedStatic) const
		{
			deconstruct(deletedStatic);
		}
};
template<> struct pTypesMap<pTypes::ID_GAME_DELETED> { typedef pGameDeleted type; };

class pObjectNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pObjectNew(RakNet::NetworkID id, unsigned int refID, unsigned int baseID, bool changed, const std::string& name, double X, double Y, double Z, double aX, double aY, double aZ, unsigned int cell, bool enabled, unsigned int lock, unsigned int owner) : pObjectNewDefault(pTypes::ID_OBJECT_NEW, id, refID, baseID)
		{
			construct(changed, name, X, Y, Z, aX, aY, aZ, cell, enabled, lock, owner);
		}
		pObjectNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& refID, unsigned int& baseID, bool& changed, std::string& name, double& X, double& Y, double& Z, double& aX, double& aY, double& aZ, unsigned int& cell, bool& enabled, unsigned int& lock, unsigned int& owner) const
		{
			deconstruct(id, refID, baseID, changed, name, X, Y, Z, aX, aY, aZ, cell, enabled, lock, owner);
		}
};
template<> struct pTypesMap<pTypes::ID_OBJECT_NEW> { typedef pObjectNew type; };

template<>
inline const typename pTypesMap<pTypes::ID_OBJECT_NEW>::type* PacketFactory::Cast_<pTypes::ID_OBJECT_NEW>::Cast(const pDefault* packet) {
	pTypes type = static_cast<pTypes>(packet->get()[0]);
	return (
		type == pTypes::ID_OBJECT_NEW ||
		type == pTypes::ID_ITEM_NEW ||
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_OBJECT_NEW>::type*>(packet) : nullptr;
}

class pItemNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pItemNew(const pPacket& _pObjectNew, RakNet::NetworkID id, unsigned int count, double condition, bool equipped, bool silent, bool stick) : pObjectNewDefault(pTypes::ID_ITEM_NEW)
		{
			construct(_pObjectNew, id, count, condition, equipped, silent, stick);
		}
		pItemNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& count, double& condition, bool& equipped, bool& silent, bool& stick) const
		{
			deconstruct(id, count, condition, equipped, silent, stick);
		}
};
template<> struct pTypesMap<pTypes::ID_ITEM_NEW> { typedef pItemNew type; };

class pContainerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pContainerNew(const pPacket& _pObjectNew, const std::vector<pPacket>& _pItemNew) : pObjectNewDefault(pTypes::ID_CONTAINER_NEW)
		{
			construct(_pObjectNew, _pItemNew);
		}
		pContainerNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(std::vector<pPacket>& _pItemNew) const
		{
			deconstruct(_pItemNew);
		}
};
template<> struct pTypesMap<pTypes::ID_CONTAINER_NEW> { typedef pContainerNew type; };

template<>
inline const typename pTypesMap<pTypes::ID_CONTAINER_NEW>::type* PacketFactory::Cast_<pTypes::ID_CONTAINER_NEW>::Cast(const pDefault* packet) {
	pTypes type = static_cast<pTypes>(packet->get()[0]);
	return (
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_CONTAINER_NEW>::type*>(packet) : nullptr;
}

class pActorNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pActorNew(const pPacket& _pContainerNew, const std::map<unsigned char, double>& values, const std::map<unsigned char, double>& baseValues, unsigned int race, signed int age, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool female, bool alerted, bool sneaking, bool dead) : pObjectNewDefault(pTypes::ID_ACTOR_NEW)
		{
			construct(_pContainerNew, values, baseValues, race, age, idle, moving, movingxy, weapon, female, alerted, sneaking, dead);
		}
		pActorNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(std::map<unsigned char, double>& values, std::map<unsigned char, double>& baseValues, unsigned int& race, signed int& age, unsigned int& idle, unsigned char& moving, unsigned char& movingxy, unsigned char& weapon, bool& female, bool& alerted, bool& sneaking, bool& dead) const
		{
			deconstruct(values, baseValues, race, age, idle, moving, movingxy, weapon, female, alerted, sneaking, dead);
		}
};
template<> struct pTypesMap<pTypes::ID_ACTOR_NEW> { typedef pActorNew type; };

template<>
inline const typename pTypesMap<pTypes::ID_ACTOR_NEW>::type* PacketFactory::Cast_<pTypes::ID_ACTOR_NEW>::Cast(const pDefault* packet) {
	pTypes type = static_cast<pTypes>(packet->get()[0]);
	return (
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_ACTOR_NEW>::type*>(packet) : nullptr;
}

class pPlayerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pPlayerNew(const pPacket& _pActorNew, const std::map<unsigned char, std::pair<unsigned char, bool>>& controls) : pObjectNewDefault(pTypes::ID_PLAYER_NEW)
		{
			construct(_pActorNew, controls);
		}
		pPlayerNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(std::map<unsigned char, std::pair<unsigned char, bool>>& controls) const
		{
			deconstruct(controls);
		}
};
template<> struct pTypesMap<pTypes::ID_PLAYER_NEW> { typedef pPlayerNew type; };

class pObjectRemove : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectRemove(RakNet::NetworkID id) : pObjectDefault(pTypes::ID_OBJECT_REMOVE, id)
		{

		}
		pObjectRemove(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id) const
		{
			deconstruct(id);
		}
};
template<> struct pTypesMap<pTypes::ID_OBJECT_REMOVE> { typedef pObjectRemove type; };

class pObjectName : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectName(RakNet::NetworkID id, const std::string& name) : pObjectDefault(pTypes::ID_UPDATE_NAME, id)
		{
			construct(name);
		}
		pObjectName(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, std::string& name) const
		{
			deconstruct(id, name);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_NAME> { typedef pObjectName type; };

class pObjectPos : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectPos(RakNet::NetworkID id, double X, double Y, double Z) : pObjectDefault(pTypes::ID_UPDATE_POS, id)
		{
			construct(X, Y, Z);
		}
		pObjectPos(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, double& X, double& Y, double& Z) const
		{
			deconstruct(id, X, Y, Z);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_POS> { typedef pObjectPos type; };

class pObjectAngle : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectAngle(RakNet::NetworkID id, unsigned char axis, double value) : pObjectDefault(pTypes::ID_UPDATE_ANGLE, id)
		{
			construct(axis, value);
		}
		pObjectAngle(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned char& axis, double& value) const
		{
			deconstruct(id, axis, value);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_ANGLE> { typedef pObjectAngle type; };

class pObjectCell : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectCell(RakNet::NetworkID id, unsigned int cell) : pObjectDefault(pTypes::ID_UPDATE_CELL, id)
		{
			construct(cell);
		}
		pObjectCell(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& cell) const
		{
			deconstruct(id, cell);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_CELL> { typedef pObjectCell type; };

class pObjectLock : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectLock(RakNet::NetworkID id, unsigned int lock) : pObjectDefault(pTypes::ID_UPDATE_LOCK, id)
		{
			construct(lock);
		}
		pObjectLock(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& lock) const
		{
			deconstruct(id, lock);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_LOCK> { typedef pObjectLock type; };

class pObjectOwner : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectOwner(RakNet::NetworkID id, unsigned int owner) : pObjectDefault(pTypes::ID_UPDATE_OWNER, id)
		{
			construct(owner);
		}
		pObjectOwner(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& owner) const
		{
			deconstruct(id, owner);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_OWNER> { typedef pObjectOwner type; };

class pItemCount : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pItemCount(RakNet::NetworkID id, unsigned int count) : pObjectDefault(pTypes::ID_UPDATE_COUNT, id)
		{
			construct(count);
		}
		pItemCount(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& count) const
		{
			deconstruct(id, count);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_COUNT> { typedef pItemCount type; };

class pItemCondition : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pItemCondition(RakNet::NetworkID id, double condition, unsigned int health) : pObjectDefault(pTypes::ID_UPDATE_CONDITION, id)
		{
			construct(condition, health);
		}
		pItemCondition(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, double& condition, unsigned int& health) const
		{
			deconstruct(id, condition, health);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_CONDITION> { typedef pItemCondition type; };

class pContainerUpdate : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pContainerUpdate(RakNet::NetworkID id, const std::pair<std::list<RakNet::NetworkID>, std::vector<pPacket>>& ndiff, const std::pair<std::list<RakNet::NetworkID>, std::vector<pPacket>>& gdiff) : pObjectDefault(pTypes::ID_UPDATE_CONTAINER, id)
		{
			construct(ndiff, gdiff);
		}
		pContainerUpdate(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, std::pair<std::list<RakNet::NetworkID>, std::vector<pPacket>>& ndiff, std::pair<std::list<RakNet::NetworkID>, std::vector<pPacket>>& gdiff) const
		{
			deconstruct(id, ndiff, gdiff);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_CONTAINER> { typedef pContainerUpdate type; };

class pActorValue : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorValue(RakNet::NetworkID id, bool base, unsigned char index, double value) : pObjectDefault(pTypes::ID_UPDATE_VALUE, id)
		{
			construct(base, index, value);
		}
		pActorValue(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, bool& base, unsigned char& index, double& value) const
		{
			deconstruct(id, base, index, value);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_VALUE> { typedef pActorValue type; };

class pActorState : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorState(RakNet::NetworkID id, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool firing) : pObjectDefault(pTypes::ID_UPDATE_STATE, id)
		{
			construct(idle, moving, movingxy, weapon, alerted, sneaking, firing);
		}
		pActorState(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& idle, unsigned char& moving, unsigned char& movingxy, unsigned char& weapon, bool& alerted, bool& sneaking, bool& firing) const
		{
			deconstruct(id, idle, moving, movingxy, weapon, alerted, sneaking, firing);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_STATE> { typedef pActorState type; };

class pActorRace : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorRace(RakNet::NetworkID id, unsigned int race, signed int age, signed int delta_age) : pObjectDefault(pTypes::ID_UPDATE_RACE, id)
		{
			construct(race, age, delta_age);
		}
		pActorRace(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& race, signed int& age, signed int& delta_age) const
		{
			deconstruct(id, race, age, delta_age);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_RACE> { typedef pActorRace type; };

class pActorSex : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorSex(RakNet::NetworkID id, bool female) : pObjectDefault(pTypes::ID_UPDATE_SEX, id)
		{
			construct(female);
		}
		pActorSex(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, bool& female) const
		{
			deconstruct(id, female);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_SEX> { typedef pActorSex type; };

class pActorDead : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorDead(RakNet::NetworkID id, bool dead, unsigned short limbs, signed char cause) : pObjectDefault(pTypes::ID_UPDATE_DEAD, id)
		{
			construct(dead, limbs, cause);
		}
		pActorDead(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, bool& dead, unsigned short& limbs, signed char& cause) const
		{
			deconstruct(id, dead, limbs, cause);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_DEAD> { typedef pActorDead type; };

class pActorFireweapon : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorFireweapon(RakNet::NetworkID id, unsigned int weapon, double attacks) : pObjectDefault(pTypes::ID_UPDATE_FIREWEAPON, id)
		{
			construct(weapon, attacks);
		}
		pActorFireweapon(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& weapon, double& attacks) const
		{
			deconstruct(id, weapon, attacks);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_FIREWEAPON> { typedef pActorFireweapon type; };

class pActorIdle : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorIdle(RakNet::NetworkID id, unsigned int idle, const std::string& name) : pObjectDefault(pTypes::ID_UPDATE_IDLE, id)
		{
			construct(idle, name);
		}
		pActorIdle(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& idle, std::string& name) const
		{
			deconstruct(id, idle, name);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_IDLE> { typedef pActorIdle type; };

class pPlayerControl : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerControl(RakNet::NetworkID id, unsigned char control, unsigned char key) : pObjectDefault(pTypes::ID_UPDATE_CONTROL, id)
		{
			construct(control, key);
		}
		pPlayerControl(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned char& control, unsigned char& key) const
		{
			deconstruct(id, control, key);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_CONTROL> { typedef pPlayerControl type; };

class pPlayerInterior : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerInterior(RakNet::NetworkID id, const std::string& cell, bool spawn) : pObjectDefault(pTypes::ID_UPDATE_INTERIOR, id)
		{
			construct(cell, spawn);
		}
		pPlayerInterior(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, std::string& cell, bool& spawn) const
		{
			deconstruct(id, cell, spawn);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_INTERIOR> { typedef pPlayerInterior type; };

class pPlayerExterior : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerExterior(RakNet::NetworkID id, unsigned int baseID, signed int x, signed int y, bool spawn) : pObjectDefault(pTypes::ID_UPDATE_EXTERIOR, id)
		{
			construct(baseID, x, y, spawn);
		}
		pPlayerExterior(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, unsigned int& baseID, signed int& x, signed int& y, bool& spawn) const
		{
			deconstruct(id, baseID, x, y, spawn);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_EXTERIOR> { typedef pPlayerExterior type; };

class pPlayerContext : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerContext(RakNet::NetworkID id, const std::array<unsigned int, 9>& context, bool spawn) : pObjectDefault(pTypes::ID_UPDATE_CONTEXT, id)
		{
			construct(context, spawn);
		}
		pPlayerContext(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, std::array<unsigned int, 9>& context, bool& spawn) const
		{
			deconstruct(id, context, spawn);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_CONTEXT> { typedef pPlayerContext type; };

class pPlayerConsole : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerConsole(RakNet::NetworkID id, bool enabled) : pObjectDefault(pTypes::ID_UPDATE_CONSOLE, id)
		{
			construct(enabled);
		}
		pPlayerConsole(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, bool& enabled) const
		{
			deconstruct(id, enabled);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_CONSOLE> { typedef pPlayerConsole type; };

class pPlayerChat : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerChat(RakNet::NetworkID id, bool enabled, bool locked, const std::pair<double, double> pos, const std::pair<double, double> size) : pObjectDefault(pTypes::ID_UPDATE_CHAT, id)
		{
			construct(enabled, locked, pos, size);
		}
		pPlayerChat(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(RakNet::NetworkID& id, bool& enabled, bool& locked, std::pair<double, double>& pos, std::pair<double, double>& size) const
		{
			deconstruct(id, enabled, locked, pos, size);
		}
};
template<> struct pTypesMap<pTypes::ID_UPDATE_CHAT> { typedef pPlayerChat type; };

#endif
