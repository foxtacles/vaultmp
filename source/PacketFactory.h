#ifndef PACKETFACTORY_H
#define PACKETFACTORY_H

#include <memory>
#include <type_traits>

#include "vaultmp.h"
#include "VaultException.h"
#include "Data.h"

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

	ID_OBJECT_NEW,
	ID_ITEM_NEW,
	ID_CONTAINER_NEW,
	ID_ACTOR_NEW,
	ID_PLAYER_NEW,
	ID_OBJECT_REMOVE,

	ID_UPDATE_POS,
	ID_UPDATE_ANGLE,
	ID_UPDATE_CELL,
	ID_UPDATE_CONTAINER,
	ID_UPDATE_VALUE,
	ID_UPDATE_STATE,
	ID_UPDATE_DEAD,
	ID_UPDATE_FIREWEAPON,
	ID_UPDATE_IDLE,
	ID_UPDATE_CONTROL,
	ID_UPDATE_INTERIOR,
	ID_UPDATE_EXTERIOR,
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

class pDefault;
typedef unique_ptr<pDefault> pPacket;

class PacketFactory
{
	private:
		PacketFactory() = delete;

		template<pTypes type, typename... Args>
		struct _Create {
			static pPacket Create(Args...);
		};

		template<pTypes type, typename... Args>
		struct _Access {
			static void Access(const pDefault* packet, Args...);
		};

	public:
		template<pTypes type, typename... Args>
		inline static pPacket Create(Args&&... args) { return _Create<type, Args...>::Create(forward<Args>(args)...); };

		template<pTypes type, typename... Args>
		inline static void Access(const pDefault* packet, Args&... args) { _Access<type, Args...>::Access(packet, forward<Args&>(args)...); };

		template<typename T>
		inline static T Pop(const pDefault* packet);

		template<typename T>
		inline static const T* packet_cast(const pDefault* packet);

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
		void unpack_tuple(const tuple<T...>&, tuple_count<N>);
		template<typename... T>
		void unpack_tuple(const tuple<T...>&, tuple_count<0>);

		template<typename... T, size_t N>
		void pack_tuple(tuple<T...>&, tuple_count<N>) const;
		template<typename... T>
		void pack_tuple(tuple<T...>&, tuple_count<0>) const;

		vector<unsigned char> data;
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
		void construct(const string&, const Args&...);

		template<typename T, typename... Args>
		void construct(const vector<T>&, const Args&...);

		template<typename T, typename... Args>
		void construct(const list<T>&, const Args&...);

		template<typename K, typename V, typename... Args>
		void construct(const map<K, V>&, const Args&...);

		template<typename T1, typename T2, typename... Args>
		void construct(const pair<T1, T2>&, const Args&...);

		template<typename... T, typename... Args>
		void construct(const tuple<T...>&, const Args&...);

		template<typename T, typename... Args>
		void deconstruct(T&, Args&...) const;
		template<typename T>
		T deconstruct_single() const;
		void deconstruct() const {}

		template<typename... Args>
		void deconstruct(string&, Args&...) const;

		template<typename T, typename... Args>
		void deconstruct(vector<T>&, Args&...) const;

		template<typename T, typename... Args>
		void deconstruct(list<T>&, Args&...) const;

		template<typename K, typename V, typename... Args>
		void deconstruct(map<K, V>&, Args&...) const;

		template<typename T1, typename T2, typename... Args>
		void deconstruct(pair<T1, T2>&, Args&...) const;

		template<typename... T, typename... Args>
		void deconstruct(tuple<T...>&, Args&...) const;

	public:
		virtual ~pDefault()
		{

		}

		const unsigned char* get() const
		{
			return &data[0];
		}

		unsigned int length() const
		{
			return data.size();
		}
};

template<typename T, typename... Args>
void pDefault::construct(const T& arg, const Args&... args)
{
	// is_trivially_copyable not implemented in GCC as of now
	static_assert(is_trivial<T>::value, "Type cannot be trivially copied");

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
void pDefault::construct(const string& arg, const Args&...args)
{
	unsigned int length = arg.length();
	const unsigned char* str = reinterpret_cast<const unsigned char*>(arg.c_str());

	data.insert(data.end(), str, str + length + 1);

	construct(args...);
}

template<typename T, typename... Args>
void pDefault::construct(const vector<T>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(args...);
}

template<typename T, typename... Args>
void pDefault::construct(const list<T>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(args...);
}

template<typename K, typename V, typename... Args>
void pDefault::construct(const map<K, V>& arg, const Args&...args)
{
	construct(arg.size());

	for (const auto& element : arg)
		construct(element);

	construct(args...);
}

template<typename T1, typename T2, typename... Args>
void pDefault::construct(const pair<T1, T2>& arg, const Args&...args)
{
	construct(arg.first);
	construct(arg.second);
	construct(args...);
}

template<typename... T, typename... Args>
void pDefault::construct(const tuple<T...>& arg, const Args&...args)
{
	unpack_tuple(arg, tuple_count<sizeof...(T) - 1>());
	construct(args...);
}

template<typename... T, size_t N>
void pDefault::unpack_tuple(const tuple<T...>& arg, tuple_count<N>)
{
	construct(std::get<N>(arg));
	unpack_tuple(arg, tuple_count<N - 1>());
}

template<typename... T>
void pDefault::unpack_tuple(const tuple<T...>& arg, tuple_count<0>)
{
	construct(std::get<0>(arg));
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
	static_assert(is_trivial<T>::value, "Type cannot be trivially copied");

	if (location + sizeof(T) > this->length())
		throw VaultException("Reading past the end of packet");

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
void pDefault::deconstruct(string& arg, Args&... args) const
{
	unsigned int length = strlen(reinterpret_cast<const char*>(&data[location]));

	if (location + length + 1 > this->length())
		throw VaultException("Reading past the end of packet");

	arg.assign(reinterpret_cast<const char*>(&data[location]), length);

	location += length + 1;

	deconstruct(args...);
}

template<typename T, typename... Args>
void pDefault::deconstruct(vector<T>& arg, Args&... args) const
{
	unsigned int size = deconstruct_single<unsigned int>();

	arg.clear();
	arg.reserve(size);

	for (unsigned int i = 0; i < size; ++i)
	{
		T data;
		deconstruct(data);
		arg.emplace_back(move(data));
	}

	deconstruct(args...);
}

template<typename T, typename... Args>
void pDefault::deconstruct(list<T>& arg, Args&... args) const
{
	unsigned int size = deconstruct_single<unsigned int>();

	arg.clear();

	for (unsigned int i = 0; i < size; ++i)
	{
		T data;
		deconstruct(data);
		arg.emplace_back(move(data));
	}

	deconstruct(args...);
}

template<typename K, typename V, typename... Args>
void pDefault::deconstruct(map<K, V>& arg, Args&... args) const
{
	unsigned int size = deconstruct_single<unsigned int>();

	arg.clear();

	for (unsigned int i = 0; i < size; ++i)
	{
		pair<K, V> data;
		deconstruct(data);
		// arg.emplace_hint(arg.end(), move(data));
		arg.insert(move(data));
	}

	deconstruct(args...);
}

template<typename T1, typename T2, typename... Args>
void pDefault::deconstruct(pair<T1, T2>& arg, Args&... args) const
{
	deconstruct(arg.first);
	deconstruct(arg.second);
	deconstruct(args...);
}

template<typename... T, typename... Args>
void pDefault::deconstruct(tuple<T...>& arg, Args&... args) const
{
	pack_tuple(arg, tuple_count<sizeof...(T) - 1>());
	deconstruct(args...);
}

template<typename... T, size_t N>
void pDefault::pack_tuple(tuple<T...>& arg, tuple_count<N>) const
{
	deconstruct(std::get<N>(arg));
	pack_tuple(arg, tuple_count<N - 1>());
}

template<typename... T>
void pDefault::pack_tuple(tuple<T...>& arg, tuple_count<0>) const
{
	deconstruct(std::get<0>(arg));
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

		pObjectDefault(pTypes type, NetworkID id) : pDefault(type)
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

		pObjectNewDefault(pTypes type, NetworkID id, unsigned int refID, unsigned int baseID) : pObjectDefault(type, id)
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
		pGameAuth(const string& name, const string& pwd) : pDefault(pTypes::ID_GAME_AUTH)
		{
			construct(name, pwd);
		}
		pGameAuth(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(string& name, string& pwd) const
		{
			deconstruct(name, pwd);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_AUTH, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameAuth(forward<Args>(args)...));
	}
};

template<>
inline const pGameAuth* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_GAME_AUTH ? reinterpret_cast<const pGameAuth*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_AUTH, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pGameAuth>(packet)->access(forward<Args&>(args)...);
	}
};

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

template<>
struct PacketFactory::_Create<pTypes::ID_GAME_LOAD> {
	inline static pPacket Create() {
		return pPacket(new pGameLoad());
	}
};

template<>
inline const pGameLoad* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_GAME_LOAD ? reinterpret_cast<const pGameLoad*>(packet) : nullptr;
}

template<>
struct PacketFactory::_Access<pTypes::ID_GAME_LOAD> {
	inline static void Access(const pDefault* packet) {
		packet_cast<pGameLoad>(packet)->access();
	}
};

class pGameMod : public pDefault
{
		friend class PacketFactory;

	private:
		pGameMod(const string& modfile, unsigned int crc) : pDefault(pTypes::ID_GAME_MOD)
		{
			construct(modfile, crc);
		}
		pGameMod(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(string& modfile, unsigned int& crc) const
		{
			deconstruct(modfile, crc);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_MOD, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameMod(forward<Args>(args)...));
	}
};

template<>
inline const pGameMod* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_GAME_MOD ? reinterpret_cast<const pGameMod*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_MOD, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pGameMod>(packet)->access(forward<Args&>(args)...);
	}
};

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

template<>
struct PacketFactory::_Create<pTypes::ID_GAME_START> {
	inline static pPacket Create() {
		return pPacket(new pGameStart());
	}
};

template<>
inline const pGameStart* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_GAME_START ? reinterpret_cast<const pGameStart*>(packet) : nullptr;
}

template<>
struct PacketFactory::_Access<pTypes::ID_GAME_START> {
	inline static void Access(const pDefault* packet) {
		packet_cast<pGameStart>(packet)->access();
	}
};

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

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_END, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameEnd(forward<Args>(args)...));
	}
};

template<>
inline const pGameEnd* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_GAME_END ? reinterpret_cast<const pGameEnd*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_END, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pGameEnd>(packet)->access(forward<Args&>(args)...);
	}
};

class pGameMessage : public pDefault
{
		friend class PacketFactory;

	private:
		pGameMessage(const string& message) : pDefault(pTypes::ID_GAME_MESSAGE)
		{
			construct(message);
		}
		pGameMessage(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(string& message) const
		{
			deconstruct(message);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_MESSAGE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameMessage(forward<Args>(args)...));
	}
};

template<>
inline const pGameMessage* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_GAME_MESSAGE ? reinterpret_cast<const pGameMessage*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_MESSAGE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pGameMessage>(packet)->access(forward<Args&>(args)...);
	}
};

class pGameChat : public pDefault
{
		friend class PacketFactory;

	private:
		pGameChat(const string& message) : pDefault(pTypes::ID_GAME_CHAT)
		{
			construct(message);
		}
		pGameChat(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{

		}

		void access(string& message) const
		{
			deconstruct(message);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_CHAT, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameChat(forward<Args>(args)...));
	}
};

template<>
inline const pGameChat* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_GAME_CHAT ? reinterpret_cast<const pGameChat*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_CHAT, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pGameChat>(packet)->access(forward<Args&>(args)...);
	}
};

class pObjectNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pObjectNew(NetworkID id, unsigned int refID, unsigned int baseID, bool changed, const string& name, double X, double Y, double Z, double aX, double aY, double aZ, unsigned int cell, bool enabled) : pObjectNewDefault(pTypes::ID_OBJECT_NEW, id, refID, baseID)
		{
			construct(changed, name, X, Y, Z, aX, aY, aZ, cell, enabled);
		}
		pObjectNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned int& refID, unsigned int& baseID, bool& changed, string& name, double& X, double& Y, double& Z, double& aX, double& aY, double& aZ, unsigned int& cell, bool& enabled) const
		{
			deconstruct(id, refID, baseID, changed, name, X, Y, Z, aX, aY, aZ, cell, enabled);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_OBJECT_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectNew(forward<Args>(args)...));
	}
};

template<>
inline const pObjectNew* PacketFactory::packet_cast(const pDefault* packet) {
	pTypes type = static_cast<pTypes>(packet->get()[0]);
	return (
		type == pTypes::ID_OBJECT_NEW ||
		type == pTypes::ID_ITEM_NEW ||
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? reinterpret_cast<const pObjectNew*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_OBJECT_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pObjectNew>(packet)->access(forward<Args&>(args)...);
	}
};

class pItemNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pItemNew(const pPacket& _pObjectNew, NetworkID id, unsigned int count, double condition, bool equipped, bool silent, bool stick) : pObjectNewDefault(pTypes::ID_ITEM_NEW)
		{
			construct(_pObjectNew, id, count, condition, equipped, silent, stick);
		}
		pItemNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned int& count, double& condition, bool& equipped, bool& silent, bool& stick) const
		{
			deconstruct(id, count, condition, equipped, silent, stick);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_ITEM_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pItemNew(forward<Args>(args)...));
	}
};

template<>
inline const pItemNew* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_ITEM_NEW ? reinterpret_cast<const pItemNew*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_ITEM_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pItemNew>(packet)->access(forward<Args&>(args)...);
	}
};

class pContainerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pContainerNew(const pPacket& _pObjectNew, const vector<pPacket>& _pItemNew) : pObjectNewDefault(pTypes::ID_CONTAINER_NEW)
		{
			construct(_pObjectNew, _pItemNew);
		}
		pContainerNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(vector<pPacket>& _pItemNew) const
		{
			deconstruct(_pItemNew);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_CONTAINER_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pContainerNew(forward<Args>(args)...));
	}
};

template<>
inline const pContainerNew* PacketFactory::packet_cast(const pDefault* packet) {
	pTypes type = static_cast<pTypes>(packet->get()[0]);
	return (
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? reinterpret_cast<const pContainerNew*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_CONTAINER_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pContainerNew>(packet)->access(forward<Args&>(args)...);
	}
};

class pActorNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pActorNew(const pPacket& _pContainerNew, const map<unsigned char, double>& values, const map<unsigned char, double>& baseValues, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool dead) : pObjectNewDefault(pTypes::ID_ACTOR_NEW)
		{
			construct(_pContainerNew, values, baseValues, idle, moving, movingxy, weapon, alerted, sneaking, dead);
		}
		pActorNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(map<unsigned char, double>& values, map<unsigned char, double>& baseValues, unsigned int& idle, unsigned char& moving, unsigned char& movingxy, unsigned char& weapon, bool& alerted, bool& sneaking, bool& dead) const
		{
			deconstruct(values, baseValues, idle, moving, movingxy, weapon, alerted, sneaking, dead);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_ACTOR_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorNew(forward<Args>(args)...));
	}
};

template<>
inline const pActorNew* PacketFactory::packet_cast(const pDefault* packet) {
	pTypes type = static_cast<pTypes>(packet->get()[0]);
	return (
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? reinterpret_cast<const pActorNew*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_ACTOR_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pActorNew>(packet)->access(forward<Args&>(args)...);
	}
};

class pPlayerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		pPlayerNew(const pPacket& _pActorNew, const map<unsigned char, pair<unsigned char, bool>>& controls) : pObjectNewDefault(pTypes::ID_PLAYER_NEW)
		{
			construct(_pActorNew, controls);
		}
		pPlayerNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{

		}

		void access(map<unsigned char, pair<unsigned char, bool>>& controls) const
		{
			deconstruct(controls);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_PLAYER_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerNew(forward<Args>(args)...));
	}
};

template<>
inline const pPlayerNew* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_PLAYER_NEW ? reinterpret_cast<const pPlayerNew*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_PLAYER_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pPlayerNew>(packet)->access(forward<Args&>(args)...);
	}
};

class pObjectRemove : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectRemove(NetworkID id) : pObjectDefault(pTypes::ID_OBJECT_REMOVE, id)
		{

		}
		pObjectRemove(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id) const
		{
			deconstruct(id);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_OBJECT_REMOVE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectRemove(forward<Args>(args)...));
	}
};

template<>
inline const pObjectRemove* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_OBJECT_REMOVE ? reinterpret_cast<const pObjectRemove*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_OBJECT_REMOVE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pObjectRemove>(packet)->access(forward<Args&>(args)...);
	}
};

class pObjectPos : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectPos(NetworkID id, double X, double Y, double Z) : pObjectDefault(pTypes::ID_UPDATE_POS, id)
		{
			construct(X, Y, Z);
		}
		pObjectPos(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, double& X, double& Y, double& Z) const
		{
			deconstruct(id, X, Y, Z);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_POS, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectPos(forward<Args>(args)...));
	}
};

template<>
inline const pObjectPos* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_POS ? reinterpret_cast<const pObjectPos*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_POS, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pObjectPos>(packet)->access(forward<Args&>(args)...);
	}
};

class pObjectAngle : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectAngle(NetworkID id, unsigned char axis, double value) : pObjectDefault(pTypes::ID_UPDATE_ANGLE, id)
		{
			construct(axis, value);
		}
		pObjectAngle(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned char& axis, double& value) const
		{
			deconstruct(id, axis, value);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_ANGLE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectAngle(forward<Args>(args)...));
	}
};

template<>
inline const pObjectAngle* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_ANGLE ? reinterpret_cast<const pObjectAngle*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_ANGLE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pObjectAngle>(packet)->access(forward<Args&>(args)...);
	}
};

class pObjectCell : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectCell(NetworkID id, unsigned int cell) : pObjectDefault(pTypes::ID_UPDATE_CELL, id)
		{
			construct(cell);
		}
		pObjectCell(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned int& cell) const
		{
			deconstruct(id, cell);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_CELL, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectCell(forward<Args>(args)...));
	}
};

template<>
inline const pObjectCell* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_CELL ? reinterpret_cast<const pObjectCell*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_CELL, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pObjectCell>(packet)->access(forward<Args&>(args)...);
	}
};

class pContainerUpdate : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pContainerUpdate(NetworkID id, const pair<list<NetworkID>, vector<pPacket>>& ndiff, const pair<list<NetworkID>, vector<pPacket>>& gdiff) : pObjectDefault(pTypes::ID_UPDATE_CONTAINER, id)
		{
			construct(ndiff, gdiff);
		}
		pContainerUpdate(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, pair<list<NetworkID>, vector<pPacket>>& ndiff, pair<list<NetworkID>, vector<pPacket>>& gdiff) const
		{
			deconstruct(id, ndiff, gdiff);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_CONTAINER, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pContainerUpdate(forward<Args>(args)...));
	}
};

template<>
inline const pContainerUpdate* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_CONTAINER ? reinterpret_cast<const pContainerUpdate*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_CONTAINER, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pContainerUpdate>(packet)->access(forward<Args&>(args)...);
	}
};

class pActorValue : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorValue(NetworkID id, bool base, unsigned char index, double value) : pObjectDefault(pTypes::ID_UPDATE_VALUE, id)
		{
			construct(base, index, value);
		}
		pActorValue(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, bool& base, unsigned char& index, double& value) const
		{
			deconstruct(id, base, index, value);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_VALUE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorValue(forward<Args>(args)...));
	}
};

template<>
inline const pActorValue* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_VALUE ? reinterpret_cast<const pActorValue*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_VALUE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pActorValue>(packet)->access(forward<Args&>(args)...);
	}
};

class pActorState : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorState(NetworkID id, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool firing) : pObjectDefault(pTypes::ID_UPDATE_STATE, id)
		{
			construct(idle, moving, movingxy, weapon, alerted, sneaking, firing);
		}
		pActorState(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned int& idle, unsigned char& moving, unsigned char& movingxy, unsigned char& weapon, bool& alerted, bool& sneaking, bool& firing) const
		{
			deconstruct(id, idle, moving, movingxy, weapon, alerted, sneaking, firing);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_STATE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorState(forward<Args>(args)...));
	}
};

template<>
inline const pActorState* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_STATE ? reinterpret_cast<const pActorState*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_STATE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pActorState>(packet)->access(forward<Args&>(args)...);
	}
};

class pActorDead : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorDead(NetworkID id, bool dead, unsigned short limbs, signed char cause) : pObjectDefault(pTypes::ID_UPDATE_DEAD, id)
		{
			construct(dead, limbs, cause);
		}
		pActorDead(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, bool& dead, unsigned short& limbs, signed char& cause) const
		{
			deconstruct(id, dead, limbs, cause);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_DEAD, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorDead(forward<Args>(args)...));
	}
};

template<>
inline const pActorDead* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_DEAD ? reinterpret_cast<const pActorDead*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_DEAD, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pActorDead>(packet)->access(forward<Args&>(args)...);
	}
};

class pActorFireweapon : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorFireweapon(NetworkID id, unsigned int weapon, double attacks) : pObjectDefault(pTypes::ID_UPDATE_FIREWEAPON, id)
		{
			construct(weapon, attacks);
		}
		pActorFireweapon(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned int& weapon, double& attacks) const
		{
			deconstruct(id, weapon, attacks);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_FIREWEAPON, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorFireweapon(forward<Args>(args)...));
	}
};

template<>
inline const pActorFireweapon* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_FIREWEAPON ? reinterpret_cast<const pActorFireweapon*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_FIREWEAPON, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pActorFireweapon>(packet)->access(forward<Args&>(args)...);
	}
};

class pActorIdle : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pActorIdle(NetworkID id, unsigned int idle, const string& name) : pObjectDefault(pTypes::ID_UPDATE_IDLE, id)
		{
			construct(idle, name);
		}
		pActorIdle(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned int& idle, string& name) const
		{
			deconstruct(id, idle, name);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_IDLE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorIdle(forward<Args>(args)...));
	}
};

template<>
inline const pActorIdle* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_IDLE ? reinterpret_cast<const pActorIdle*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_IDLE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pActorIdle>(packet)->access(forward<Args&>(args)...);
	}
};

class pPlayerControl : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerControl(NetworkID id, unsigned char control, unsigned char key) : pObjectDefault(pTypes::ID_UPDATE_CONTROL, id)
		{
			construct(control, key);
		}
		pPlayerControl(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned char& control, unsigned char& key) const
		{
			deconstruct(id, control, key);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_CONTROL, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerControl(forward<Args>(args)...));
	}
};

template<>
inline const pPlayerControl* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_CONTROL ? reinterpret_cast<const pPlayerControl*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_CONTROL, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pPlayerControl>(packet)->access(forward<Args&>(args)...);
	}
};

class pPlayerInterior : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerInterior(NetworkID id, const string& cell, bool spawn) : pObjectDefault(pTypes::ID_UPDATE_INTERIOR, id)
		{
			construct(cell, spawn);
		}
		pPlayerInterior(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, string& cell, bool& spawn) const
		{
			deconstruct(id, cell, spawn);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_INTERIOR, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerInterior(forward<Args>(args)...));
	}
};

template<>
inline const pPlayerInterior* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_INTERIOR ? reinterpret_cast<const pPlayerInterior*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_INTERIOR, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pPlayerInterior>(packet)->access(forward<Args&>(args)...);
	}
};

class pPlayerExterior : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerExterior(NetworkID id, unsigned int baseID, signed int x, signed int y, bool spawn) : pObjectDefault(pTypes::ID_UPDATE_EXTERIOR, id)
		{
			construct(baseID, x, y, spawn);
		}
		pPlayerExterior(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{

		}

		void access(NetworkID& id, unsigned int& baseID, signed int& x, signed int& y, bool& spawn) const
		{
			deconstruct(id, baseID, x, y, spawn);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_EXTERIOR, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerExterior(forward<Args>(args)...));
	}
};

template<>
inline const pPlayerExterior* PacketFactory::packet_cast(const pDefault* packet) {
	return static_cast<pTypes>(packet->get()[0]) == pTypes::ID_UPDATE_EXTERIOR ? reinterpret_cast<const pPlayerExterior*>(packet) : nullptr;
}

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_EXTERIOR, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		packet_cast<pPlayerExterior>(packet)->access(forward<Args&>(args)...);
	}
};

#endif
