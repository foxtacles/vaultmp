#ifndef PACKETFACTORY_H
#define PACKETFACTORY_H

#include <memory>

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
	ID_GAME_AUTH = Data::ID_GAME_FIRST,
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

	ID_OBJECT_UPDATE,
	ID_CONTAINER_UPDATE,
	ID_ACTOR_UPDATE,
	ID_PLAYER_UPDATE,

	ID_UPDATE_POS,
	ID_UPDATE_ANGLE,
	ID_UPDATE_CELL,
	ID_UPDATE_CONTAINER,
	ID_UPDATE_VALUE,
	ID_UPDATE_STATE,
	ID_UPDATE_DEAD,
	ID_UPDATE_FIREWEAPON,
	ID_UPDATE_CONTROL,
	ID_UPDATE_INTERIOR,
	ID_UPDATE_EXTERIOR,

	ID_REASON_KICK,
	ID_REASON_BAN,
	ID_REASON_ERROR,
	ID_REASON_DENIED,
	ID_REASON_NONE,
};

class pDefault;
typedef unique_ptr<pDefault, void(*)(pDefault*)> pPacket;

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

		static pPacket Init(const unsigned char* stream, unsigned int len);

		static NetworkID ExtractNetworkID(const pDefault* packet);
		static unsigned int ExtractReference(const pDefault* packet);
		static unsigned int ExtractBase(const pDefault* packet);
		static const unsigned char* ExtractRawData(const pDefault* packet);
		static pPacket ExtractPartial(const pDefault* packet);
		static void FreePacket(pDefault* packet);
};

#pragma pack(push, 1)
struct pTypeSpecifier
{
	pTypes type;
};
#pragma pack(pop)

class pDefault
{
		friend class PacketFactory;

	private:
		pDefault(const pDefault&) = delete;
		pDefault& operator=(const pDefault&) = delete;

	protected:
		pDefault(pTypes type)
		{
			this->type.type = type;
			this->stream = nullptr;
			this->len = 0;
			base();
		};

		pDefault(const unsigned char* stream, unsigned int len)
		{
			this->stream = new unsigned char[len];
			memcpy(this->stream, stream, len);
			this->len = len;
			base();
		};

		pTypeSpecifier type;
		unsigned char* stream;
		unsigned int len;
		unsigned int base_len;

		virtual void construct(void*, unsigned int) = 0;
		virtual void deconstruct(void*, unsigned int) = 0;

		void base()
		{
			base_len = sizeof(pTypeSpecifier);
		};

	public:
		virtual ~pDefault()
		{
			delete[] stream;
		};

		const unsigned char* get() const
		{
			return stream;
		};

		unsigned int length() const
		{
			return len;
		};

		unsigned int base_length() const
		{
			return base_len;
		};
};

class pGameDefault : public pDefault
{
		friend class PacketFactory;

	protected:
		pGameDefault(pTypes type) : pDefault(type)
		{
			base();
		};

		pGameDefault(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{
			base();
		};

		void base()
		{

		};

		virtual void construct(void* super = nullptr, unsigned int len = 0)
		{
			if (!stream)
			{
				unsigned int written = 0;
				stream = new unsigned char[base_len + len];
				memcpy(stream, &type, sizeof(pTypeSpecifier));
				written += sizeof(pTypeSpecifier);
				memcpy((void*)(stream + written), super, len);
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct(void* super = nullptr, unsigned int len = 0)
		{
			if (stream)
			{
				if (base_len + len != this->len)
					throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, base_len + len);

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>(stream);
				read += sizeof(pTypeSpecifier);

				if (super)
					memcpy(super, (void*)(stream + read), len);
			}
		};
};

class pObjectDefault : public pDefault
{
		friend class PacketFactory;

	protected:
		pObjectDefault(pTypes type, NetworkID id) : pDefault(type)
		{
			this->id = id;
			base();
		};

		pObjectDefault(const unsigned char* stream, unsigned int len) : pDefault(stream, len)
		{
			base();
		};

		NetworkID id;

		void base()
		{
			base_len += sizeof(NetworkID);
		};

		virtual void construct(void* super = nullptr, unsigned int len = 0)
		{
			if (!stream)
			{
				unsigned int written = 0;
				stream = new unsigned char[base_len + len];
				memcpy(stream, &type, sizeof(pTypeSpecifier));
				written += sizeof(pTypeSpecifier);
				memcpy((void*)(stream + written), &id, sizeof(NetworkID));
				written += sizeof(NetworkID);
				memcpy((void*)(stream + written), super, len);
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct(void* super = nullptr, unsigned int len = 0)
		{
			if (stream)
			{
				if (base_len + len != this->len)
					throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, base_len + len);

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>(stream);
				read += sizeof(pTypeSpecifier);
				id = *reinterpret_cast<NetworkID*>(stream + read);
				read += sizeof(NetworkID);

				if (super)
					memcpy(super, (void*)(stream + read), len);
			}
		};
};

class pObjectNewDefault : public pObjectDefault
{
		friend class PacketFactory;

	protected:
		pObjectNewDefault(pTypes type, unsigned int refID, unsigned int baseID, NetworkID id) : pObjectDefault(type, id)
		{
			this->refID = refID;
			this->baseID = baseID;
			base();
		};

		pObjectNewDefault(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{
			base();
		};

		unsigned int refID;
		unsigned int baseID;

		void base()
		{
			base_len += sizeof(unsigned int);
			base_len += sizeof(unsigned int);
		};

		virtual void construct(void* super = nullptr, unsigned int len = 0)
		{
			if (!stream)
			{
				unsigned int written = 0;
				stream = new unsigned char[base_len + len];
				memcpy(stream, &type, sizeof(pTypeSpecifier));
				written += sizeof(pTypeSpecifier);
				memcpy((void*)(stream + written), &id, sizeof(NetworkID));
				written += sizeof(NetworkID);
				memcpy((void*)(stream + written), &refID, sizeof(unsigned int));
				written += sizeof(unsigned int);
				memcpy((void*)(stream + written), &baseID, sizeof(unsigned int));
				written += sizeof(unsigned int);
				memcpy((void*)(stream + written), super, len);
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct(void* super = nullptr, unsigned int len = 0)
		{
			if (stream)
			{
				if (base_len + len != this->len)
					throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, base_len + len);

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>(stream);
				read += sizeof(pTypeSpecifier);
				id = *reinterpret_cast<NetworkID*>(stream + read);
				read += sizeof(NetworkID);
				refID = *reinterpret_cast<unsigned int*>(stream + read);
				read += sizeof(unsigned int);
				baseID = *reinterpret_cast<unsigned int*>(stream + read);
				read += sizeof(unsigned int);

				if (super)
					memcpy(super, (void*)(stream + read), len);
			}
		};
};

class pObjectUpdateDefault : public pObjectDefault
{
		friend class PacketFactory;

	protected:
		pObjectUpdateDefault(pTypes type, pTypes sub_type, NetworkID id) : pObjectDefault(type, id)
		{
			this->sub_type.type = sub_type;
			base();
		};

		pObjectUpdateDefault(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{
			base();
		};

		pTypeSpecifier sub_type;

		void base()
		{
			base_len += sizeof(pTypeSpecifier);
		};

		virtual void construct(void* super = nullptr, unsigned int len = 0)
		{
			if (!stream)
			{
				unsigned int written = 0;
				stream = new unsigned char[sizeof(pTypeSpecifier) + sizeof(pTypeSpecifier) + sizeof(NetworkID) + len];
				memcpy(stream, &type, sizeof(pTypeSpecifier));
				written += sizeof(pTypeSpecifier);
				memcpy((void*)(stream + written), &sub_type, sizeof(pTypeSpecifier));
				written += sizeof(pTypeSpecifier);
				memcpy((void*)(stream + written), &id, sizeof(NetworkID));
				written += sizeof(NetworkID);
				memcpy((void*)(stream + written), super, len);
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct(void* super = nullptr, unsigned int len = 0)
		{
			if (stream)
			{
				if (base_len + len != this->len)
					throw VaultException("Packet has size %d instead of expected %d,%d bytes!", this->len, base_len , len);

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>(stream);
				read += sizeof(pTypeSpecifier);
				sub_type = *reinterpret_cast<pTypeSpecifier*>(stream + read);
				read += sizeof(pTypeSpecifier);
				id = *reinterpret_cast<NetworkID*>(stream + read);
				read += sizeof(NetworkID);

				if (super)
					memcpy(super, (void*)(stream + read), len);
			}
		};
};

/* ************************************** */

// Real types

/* ************************************** */

#pragma pack(push, 1)
struct _pGameAuth
{
	char name[MAX_PLAYER_NAME];
	char pwd[MAX_PASSWORD_SIZE];
};

struct _pGameMod
{
	char modfile[MAX_MOD_FILE];
	unsigned int crc;
};
#pragma pack(pop)

class pGameAuth : public pGameDefault
{
		friend class PacketFactory;

	private:
		_pGameAuth _data;

		pGameAuth(const char* name, const char* pwd) : pGameDefault(pTypes::ID_GAME_AUTH)
		{
			strncpy(_data.name, name, sizeof(_data.name));
			strncpy(_data.pwd, pwd, sizeof(_data.pwd));
			construct(&_data, sizeof(_data));
		}
		pGameAuth(const unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(string& name, string& pwd) const
		{
			name.assign(_data.name, find(_data.name, _data.name + sizeof(_data.name), 0));
			pwd.assign(_data.pwd, find(_data.pwd, _data.pwd + sizeof(_data.pwd), 0));
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_AUTH, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameAuth(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_AUTH, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pGameAuth&>(*packet).access(forward<Args&>(args)...);
	}
};

class pGameLoad : public pGameDefault
{
		friend class PacketFactory;

	private:
		pGameLoad() : pGameDefault(pTypes::ID_GAME_LOAD)
		{
			construct();
		}
		pGameLoad(const unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
		{
			deconstruct();
		}

		void access() const
		{

		}
};

template<>
struct PacketFactory::_Create<pTypes::ID_GAME_LOAD> {
	inline static pPacket Create() {
		return pPacket(new pGameLoad(), FreePacket);
	}
};

template<>
struct PacketFactory::_Access<pTypes::ID_GAME_LOAD> {
	inline static void Access(const pDefault* packet) {
		dynamic_cast<const pGameLoad&>(*packet).access();
	}
};

class pGameMod : public pGameDefault
{
		friend class PacketFactory;

	private:
		_pGameMod _data;

		pGameMod(const string& modfile, unsigned int crc) : pGameDefault(pTypes::ID_GAME_MOD)
		{
			strncpy(_data.modfile, modfile.c_str(), sizeof(_data.modfile));
			_data.crc = crc;
			construct(&_data, sizeof(_data));
		}
		pGameMod(const unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(string& modfile, unsigned int& crc) const
		{
			modfile.assign(_data.modfile, find(_data.modfile, _data.modfile + sizeof(_data.modfile), 0));
			crc = _data.crc;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_MOD, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameMod(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_MOD, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pGameMod&>(*packet).access(forward<Args&>(args)...);
	}
};

class pGameStart : public pGameDefault
{
		friend class PacketFactory;

	private:
		pGameStart() : pGameDefault(pTypes::ID_GAME_START)
		{
			construct();
		}
		pGameStart(const unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
		{
			deconstruct();
		}

		void access() const
		{

		}
};

template<>
struct PacketFactory::_Create<pTypes::ID_GAME_START> {
	inline static pPacket Create() {
		return pPacket(new pGameStart(), FreePacket);
	}
};

template<>
struct PacketFactory::_Access<pTypes::ID_GAME_START> {
	inline static void Access(const pDefault* packet) {
		dynamic_cast<const pGameStart&>(*packet).access();
	}
};

class pGameEnd : public pGameDefault
{
		friend class PacketFactory;

	private:
		pTypeSpecifier reason;

		pGameEnd(pTypes reason) : pGameDefault(pTypes::ID_GAME_END)
		{
			this->reason.type = reason;
			construct(&reason, sizeof(reason));
		}
		pGameEnd(const unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
		{
			deconstruct(&reason, sizeof(reason));
		}

		void access(pTypes& reason) const
		{
			reason = this->reason.type;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_END, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameEnd(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_END, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pGameEnd&>(*packet).access(forward<Args&>(args)...);
	}
};

class pGameMessage : public pGameDefault
{
		friend class PacketFactory;

	private:
		char message[MAX_MESSAGE_LENGTH];

		pGameMessage(const string& message) : pGameDefault(pTypes::ID_GAME_MESSAGE)
		{
			strncpy(this->message, message.c_str(), sizeof(this->message));
			construct(this->message, sizeof(this->message));
		}
		pGameMessage(const unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
		{
			deconstruct(this->message, sizeof(this->message));
		}

		void access(string& message) const
		{
			message.assign(this->message, find(this->message, this->message + sizeof(this->message), 0));
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_MESSAGE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameMessage(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_MESSAGE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pGameMessage&>(*packet).access(forward<Args&>(args)...);
	}
};

class pGameChat : public pGameDefault
{
		friend class PacketFactory;

	private:
		char message[MAX_CHAT_LENGTH];

		pGameChat(const string& message) : pGameDefault(pTypes::ID_GAME_CHAT)
		{
			strncpy(this->message, message.c_str(), sizeof(this->message));
			construct(this->message, sizeof(this->message));
		}
		pGameChat(const unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
		{
			deconstruct(this->message, sizeof(this->message));
		}

		void access(string& message) const
		{
			message.assign(this->message, find(this->message, this->message + sizeof(this->message), 0));
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_GAME_CHAT, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pGameChat(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_GAME_CHAT, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pGameChat&>(*packet).access(forward<Args&>(args)...);
	}
};

/* ************************************** */

#pragma pack(push, 1)
struct _pObjectNew
{
	char name[MAX_PLAYER_NAME];
	double X;
	double Y;
	double Z;
	double aX;
	double aY;
	double aZ;
	unsigned int cell;
	bool enabled;
};

struct _pItemNew
{
	_pObjectNew _data_pObjectNew;
	unsigned int count;
	double condition;
	bool equipped;
	bool silent;
	bool stick;
};
#pragma pack(pop)

class pObjectNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		_pObjectNew _data;

		pObjectNew(NetworkID id, unsigned int refID, unsigned int baseID, const char* name, double X, double Y, double Z, double aX, double aY, double aZ, unsigned int cell, bool enabled) : pObjectNewDefault(pTypes::ID_OBJECT_NEW, refID, baseID, id)
		{
			strncpy(this->_data.name, name, sizeof(this->_data.name));
			_data.X = X;
			_data.Y = Y;
			_data.Z = Z;
			_data.aX = aX;
			_data.aY = aY;
			_data.aZ = aZ;
			_data.cell = cell;
			_data.enabled = enabled;
			construct(&_data, sizeof(_data));
		}
		pObjectNew(NetworkID id, unsigned int refID, unsigned int baseID, const _pObjectNew& data) : pObjectNewDefault(pTypes::ID_OBJECT_NEW, refID, baseID, id)
		{
			_data = data;
			construct(&_data, sizeof(_data));
		}
		pObjectNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, unsigned int& refID, unsigned int& baseID, string& name, double& X, double& Y, double& Z, double& aX, double& aY, double& aZ, unsigned int& cell, bool& enabled) const
		{
			id = this->id;
			refID = this->refID;
			baseID = this->baseID;
			name.assign(_data.name, find(_data.name, _data.name + sizeof(_data.name), 0));
			X = _data.X;
			Y = _data.Y;
			Z = _data.Z;
			aX = _data.aX;
			aY = _data.aY;
			aZ = _data.aZ;
			cell = _data.cell;
			enabled = _data.enabled;
		}

	public:
		static unsigned int data_length()
		{
			return sizeof(_pObjectNew);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_OBJECT_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectNew(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_OBJECT_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pObjectNew&>(*packet).access(forward<Args&>(args)...);
	}
};

class pItemNew : public pObjectNewDefault
{
		friend class PacketFactory;
		friend class pContainerNew;
		friend class pContainerUpdate;

	private:
		_pItemNew _data;

		pItemNew(const pDefault* _data_pObjectNew, unsigned int count, double condition, bool equipped, bool silent, bool stick) : pObjectNewDefault(pTypes::ID_ITEM_NEW, PacketFactory::ExtractReference(_data_pObjectNew), PacketFactory::ExtractBase(_data_pObjectNew), PacketFactory::ExtractNetworkID(_data_pObjectNew))
		{
			memcpy(&this->_data._data_pObjectNew, PacketFactory::ExtractRawData(_data_pObjectNew), pObjectNew::data_length());
			_data.count = count;
			_data.condition = condition;
			_data.equipped = equipped;
			_data.silent = silent;
			_data.stick = stick;
			construct(&_data, sizeof(_data));
		}
		pItemNew(NetworkID id, unsigned int refID, unsigned int baseID, _pItemNew& data) : pObjectNewDefault(pTypes::ID_ITEM_NEW, refID, baseID, id)
		{
			_data = data;
			construct(&_data, sizeof(_data));
		}
		pItemNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(unsigned int& count, double& condition, bool& equipped, bool& silent, bool& stick) const
		{
			count = _data.count;
			condition = _data.condition;
			equipped = _data.equipped;
			silent = _data.silent;
			stick = _data.stick;
		}

	public:
		static unsigned int data_length()
		{
			return sizeof(_pItemNew);
		}

		// I need a design solution for that

		static unsigned int as_packet_length()
		{
			return sizeof(pTypeSpecifier) + sizeof(NetworkID) + sizeof(unsigned int) + sizeof(unsigned int) + sizeof(_pItemNew);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_ITEM_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pItemNew(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_ITEM_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pItemNew&>(*packet).access(forward<Args&>(args)...);
	}
};

class pContainerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		unsigned char* _data;

		pContainerNew(const pDefault* _data_pObjectNew, const vector<pPacket>& _data_pItemNew) : pObjectNewDefault(pTypes::ID_CONTAINER_NEW, PacketFactory::ExtractReference(_data_pObjectNew), PacketFactory::ExtractBase(_data_pObjectNew), PacketFactory::ExtractNetworkID(_data_pObjectNew))
		{
			unsigned int at = 0;
			unsigned int length = pItemNew::as_packet_length();
			unsigned int size = _data_pItemNew.size();
			unsigned int mem = pObjectNew::data_length() + (length * size) + sizeof(unsigned int);
			_data = new unsigned char[mem];

			memcpy(&this->_data[0], PacketFactory::ExtractRawData(_data_pObjectNew), pObjectNew::data_length());
			at += pObjectNew::data_length();

			memcpy(&this->_data[at], &size, sizeof(unsigned int));
			at += sizeof(unsigned int);

			if (size > 0)
			{
				vector<pPacket>::const_iterator it;

				for (it = _data_pItemNew.begin(); it != _data_pItemNew.end(); ++it, at += length)
				{
					const pDefault* packet = it->get();

					if (packet->length() != length)
					{
						delete[] _data;
						throw VaultException("Packet has size %d instead of expected %d bytes!", packet->length(), length);
					}

					memcpy(&_data[at], packet->get(), length);
				}
			}

			construct(_data, mem);
		}
		pContainerNew(NetworkID id, unsigned int refID, unsigned int baseID, const unsigned char* data) : pObjectNewDefault(pTypes::ID_CONTAINER_NEW, refID, baseID, id)
		{
			unsigned int mem = pContainerNew::data_length(data);
			_data = new unsigned char[mem];
			memcpy(_data, data, mem);

			construct(_data, mem);
		}
		pContainerNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{
			unsigned int mem = len - this->base_length();
			_data = new unsigned char[mem];

			deconstruct(_data, mem);
		}
		virtual ~pContainerNew()
		{
			delete[] _data;
		}

		void access(vector<pPacket>& _pItemNew) const
		{
			_pItemNew.clear();
			unsigned int length = pItemNew::as_packet_length();
			unsigned int size = *reinterpret_cast<unsigned int*>(&_data[pObjectNew::data_length()]);
			unsigned int at = pObjectNew::data_length() + sizeof(unsigned int);

			_pItemNew.reserve(size);
			for (unsigned int i = 0; i < size; ++i, at += length)
				_pItemNew.emplace_back(new pItemNew(&_data[at], length), PacketFactory::FreePacket);
		}

	public:
		static unsigned int data_length(const unsigned char* data)
		{
			unsigned int length = pItemNew::as_packet_length();
			unsigned int size = *reinterpret_cast<const unsigned int*>(&data[pObjectNew::data_length()]);
			unsigned int mem = pObjectNew::data_length() + (length * size) + sizeof(unsigned int);

			return mem;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_CONTAINER_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pContainerNew(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_CONTAINER_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pContainerNew&>(*packet).access(forward<Args&>(args)...);
	}
};

class pActorNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		unsigned char* _data;

		pActorNew(const pDefault* _data_pContainerNew, const map<unsigned char, double>& values, const map<unsigned char, double>& baseValues, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool dead) : pObjectNewDefault(pTypes::ID_ACTOR_NEW, PacketFactory::ExtractReference(_data_pContainerNew), PacketFactory::ExtractBase(_data_pContainerNew), PacketFactory::ExtractNetworkID(_data_pContainerNew))
		{
			unsigned int at = 0;
			unsigned int length = sizeof(unsigned char) + sizeof(double);

			if (values.size() != baseValues.size())
				throw VaultException("Lengths of values / base values of an Actor must be equal!");

			unsigned int size = values.size();
			unsigned int container_length = pContainerNew::data_length(PacketFactory::ExtractRawData(_data_pContainerNew));
			unsigned int mem = container_length + (length * size * 2) + sizeof(unsigned int) + (sizeof(unsigned char) * 3) + (sizeof(bool) * 3);
			_data = new unsigned char[mem];

			memcpy(&this->_data[0], PacketFactory::ExtractRawData(_data_pContainerNew), container_length);
			at += container_length;

			memcpy(&this->_data[at], &size, sizeof(unsigned int));
			at += sizeof(unsigned int);

			if (size > 0)
			{
				map<unsigned char, double>::const_iterator it;

				for (it = values.begin(); it != values.end(); ++it, at += length)
				{
					memcpy(&_data[at], &it->first, sizeof(unsigned char));
					memcpy(&_data[at + sizeof(unsigned char)], &it->second, sizeof(double));
				}

				for (it = baseValues.begin(); it != baseValues.end(); ++it, at += length)
				{
					memcpy(&_data[at], &it->first, sizeof(unsigned char));
					memcpy(&_data[at + sizeof(unsigned char)], &it->second, sizeof(double));
				}
			}

			memcpy(&_data[at], &moving, sizeof(unsigned char));
			at += sizeof(unsigned char);

			memcpy(&_data[at], &movingxy, sizeof(unsigned char));
			at += sizeof(unsigned char);

			memcpy(&_data[at], &weapon, sizeof(unsigned char));
			at += sizeof(unsigned char);

			memcpy(&_data[at], &alerted, sizeof(bool));
			at += sizeof(bool);

			memcpy(&_data[at], &sneaking, sizeof(bool));
			at += sizeof(bool);

			memcpy(&_data[at], &dead, sizeof(bool));
			at += sizeof(bool);

			construct(_data, mem);
		}
		pActorNew(NetworkID id, unsigned int refID, unsigned int baseID, const unsigned char* data) : pObjectNewDefault(pTypes::ID_ACTOR_NEW, refID, baseID, id)
		{
			unsigned int mem = pActorNew::data_length(data);
			_data = new unsigned char[mem];
			memcpy(_data, data, mem);

			construct(_data, mem);
		}
		pActorNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{
			unsigned int mem = len - this->base_length();
			_data = new unsigned char[mem];

			deconstruct(_data, mem);
		}
		virtual ~pActorNew()
		{
			delete[] _data;
		}

		void access(map<unsigned char, double>& values, map<unsigned char, double>& baseValues, unsigned char& moving, unsigned char& movingxy, unsigned char& weapon, bool& alerted, bool& sneaking, bool& dead) const
		{
			values.clear();
			baseValues.clear();
			unsigned int length = sizeof(unsigned char) + sizeof(double);
			unsigned int size = *reinterpret_cast<unsigned int*>(&_data[pContainerNew::data_length(_data)]);
			unsigned int at = pContainerNew::data_length(_data) + sizeof(unsigned int);

			for (unsigned int i = 0; i < size; ++i, at += length)
				values.insert(make_pair(*reinterpret_cast<unsigned char*>(&_data[at]), *reinterpret_cast<double*>(&_data[at + sizeof(unsigned char)])));

			for (unsigned int i = 0; i < size; ++i, at += length)
				baseValues.insert(make_pair(*reinterpret_cast<unsigned char*>(&_data[at]), *reinterpret_cast<double*>(&_data[at + sizeof(unsigned char)])));

			moving = *reinterpret_cast<unsigned char*>(&_data[at]);
			at += sizeof(unsigned char);

			movingxy = *reinterpret_cast<unsigned char*>(&_data[at]);
			at += sizeof(unsigned char);

			weapon = *reinterpret_cast<unsigned char*>(&_data[at]);
			at += sizeof(unsigned char);

			alerted = *reinterpret_cast<bool*>(&_data[at]);
			at += sizeof(bool);

			sneaking = *reinterpret_cast<bool*>(&_data[at]);
			at += sizeof(bool);

			dead = *reinterpret_cast<bool*>(&_data[at]);
		}

	public:
		static unsigned int data_length(const unsigned char* data)
		{
			unsigned int mem = pContainerNew::data_length(data);
			unsigned int length = sizeof(unsigned char) + sizeof(double);
			unsigned int size = *reinterpret_cast<const unsigned int*>(&data[mem]);
			mem += (length * size * 2) + sizeof(unsigned int) + (sizeof(unsigned char) * 3) + (sizeof(bool) * 3);;

			return mem;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_ACTOR_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorNew(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_ACTOR_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pActorNew&>(*packet).access(forward<Args&>(args)...);
	}
};

class pPlayerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		unsigned char* _data;

		pPlayerNew(const pDefault* _data_pActorNew, const map<unsigned char, pair<unsigned char, bool>>& controls) : pObjectNewDefault(pTypes::ID_PLAYER_NEW, PacketFactory::ExtractReference(_data_pActorNew), PacketFactory::ExtractBase(_data_pActorNew), PacketFactory::ExtractNetworkID(_data_pActorNew))
		{
			unsigned int at = 0;
			unsigned int length = sizeof(unsigned char) + sizeof(unsigned char) + sizeof(bool);
			unsigned int size = controls.size();
			unsigned int actor_length = pActorNew::data_length(PacketFactory::ExtractRawData(_data_pActorNew));
			unsigned int mem = actor_length + (length * size) + sizeof(unsigned int);
			_data = new unsigned char[mem];

			memcpy(&this->_data[0], PacketFactory::ExtractRawData(_data_pActorNew), actor_length);
			at += actor_length;

			memcpy(&this->_data[at], &size, sizeof(unsigned int));
			at += sizeof(unsigned int);

			if (size > 0)
			{
				map<unsigned char, pair<unsigned char, bool>>::const_iterator it;

				for (it = controls.begin(); it != controls.end(); ++it, at += length)
				{
					memcpy(&_data[at], &it->first, sizeof(unsigned char));
					memcpy(&_data[at + sizeof(unsigned char)], &it->second.first, sizeof(unsigned char));
					memcpy(&_data[at + (sizeof(unsigned char) * 2)], &it->second.second, sizeof(bool));
				}
			}

			construct(_data, mem);
		}
		pPlayerNew(const unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
		{
			unsigned int mem = len - this->base_length();
			_data = new unsigned char[mem];

			deconstruct(_data, mem);
		}
		virtual ~pPlayerNew()
		{
			delete[] _data;
		}

		void access(map<unsigned char, pair<unsigned char, bool>>& controls) const
		{
			controls.clear();
			unsigned int length = sizeof(unsigned char) + sizeof(unsigned char) + sizeof(bool);
			unsigned int size = *reinterpret_cast<unsigned int*>(&_data[pActorNew::data_length(_data)]);
			unsigned int at = pActorNew::data_length(_data) + sizeof(unsigned int);

			for (unsigned int i = 0; i < size; ++i, at += length)
				controls.insert(make_pair(*reinterpret_cast<unsigned char*>(&_data[at]), make_pair(*reinterpret_cast<double*>(&_data[at + sizeof(unsigned char)]), *reinterpret_cast<double*>(&_data[at + (sizeof(unsigned char) * 2)]))));
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_PLAYER_NEW, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerNew(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_PLAYER_NEW, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pPlayerNew&>(*packet).access(forward<Args&>(args)...);
	}
};

class pObjectRemove : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectRemove(NetworkID id) : pObjectDefault(pTypes::ID_OBJECT_REMOVE, id)
		{
			construct();
		}
		pObjectRemove(const unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
		{
			deconstruct();
		}

		void access(NetworkID& id) const
		{
			id = this->id;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_OBJECT_REMOVE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectRemove(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_OBJECT_REMOVE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pObjectRemove&>(*packet).access(forward<Args&>(args)...);
	}
};

/* ************************************** */

#pragma pack(push, 1)
struct _pObjectPos
{
	double X;
	double Y;
	double Z;
};

struct _pObjectAngle
{
	unsigned char axis;
	double value;
};
#pragma pack(pop)

class pObjectPos : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pObjectPos _data;

		pObjectPos(NetworkID id, double X, double Y, double Z) : pObjectUpdateDefault(pTypes::ID_OBJECT_UPDATE, pTypes::ID_UPDATE_POS, id)
		{
			_data.X = X;
			_data.Y = Y;
			_data.Z = Z;
			construct(&_data, sizeof(_data));
		}
		pObjectPos(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, double& X, double& Y, double& Z) const
		{
			id = this->id;
			X = _data.X;
			Y = _data.Y;
			Z = _data.Z;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_POS, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectPos(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_POS, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pObjectPos&>(*packet).access(forward<Args&>(args)...);
	}
};

class pObjectAngle : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pObjectAngle _data;

		pObjectAngle(NetworkID id, unsigned char axis, double value) : pObjectUpdateDefault(pTypes::ID_OBJECT_UPDATE, pTypes::ID_UPDATE_ANGLE, id)
		{
			_data.axis = axis;
			_data.value = value;
			construct(&_data, sizeof(_data));
		}
		pObjectAngle(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, unsigned char& axis, double& value) const
		{
			id = this->id;
			axis = _data.axis;
			value = _data.value;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_ANGLE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectAngle(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_ANGLE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pObjectAngle&>(*packet).access(forward<Args&>(args)...);
	}
};

class pObjectCell : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		unsigned int cell;

		pObjectCell(NetworkID id, unsigned int cell) : pObjectUpdateDefault(pTypes::ID_OBJECT_UPDATE, pTypes::ID_UPDATE_CELL, id)
		{
			this->cell = cell;
			construct(&this->cell, sizeof(this->cell));
		}
		pObjectCell(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&this->cell, sizeof(this->cell));
		}

		void access(NetworkID& id, unsigned int& cell) const
		{
			id = this->id;
			cell = this->cell;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_CELL, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pObjectCell(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_CELL, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pObjectCell&>(*packet).access(forward<Args&>(args)...);
	}
};

/* ************************************** */

class pContainerUpdate : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		unsigned char* _data;

		pContainerUpdate(NetworkID id, const pair<list<NetworkID>, vector<pPacket>>& diff) : pObjectUpdateDefault(pTypes::ID_CONTAINER_UPDATE, pTypes::ID_UPDATE_CONTAINER, id)
		{
			unsigned int at = 0;
			unsigned int length = sizeof(NetworkID);
			unsigned int size = diff.first.size();
			unsigned int mem = (length * size) + sizeof(unsigned int);

			unsigned int length2 = pItemNew::as_packet_length();
			unsigned int size2 = diff.second.size();
			mem += (length2 * size2) + sizeof(unsigned int);
			_data = new unsigned char[mem];

			memcpy(&this->_data[at], &size, sizeof(unsigned int));
			at += sizeof(unsigned int);

			for (auto it = diff.first.begin(); it != diff.first.end(); ++it)
			{
				memcpy(&this->_data[at], &(*it), length);
				at += length;
			}

			memcpy(&this->_data[at], &size2, sizeof(unsigned int));
			at += sizeof(unsigned int);

			for (auto it = diff.second.begin(); it != diff.second.end(); ++it)
			{
				memcpy(&this->_data[at], it->get(), length2);
				at += length2;
			}

			construct(_data, mem);
		}
		pContainerUpdate(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			unsigned int mem = len - this->base_length();
			_data = new unsigned char[mem];

			deconstruct(_data, mem);
		}
		virtual ~pContainerUpdate()
		{
			delete[] _data;
		}

		void access(NetworkID& id, pair<list<NetworkID>, vector<pPacket>>& diff) const
		{
			id = this->id;

			diff.first.clear();
			diff.second.clear();
			unsigned int length = sizeof(NetworkID);
			unsigned int size = *reinterpret_cast<unsigned int*>(&_data[0]);
			unsigned int at = sizeof(unsigned int);

			for (unsigned int i = 0; i < size; ++i, at += length)
				diff.first.emplace_back(*reinterpret_cast<NetworkID*>(&_data[at]));

			length = pItemNew::as_packet_length();
			size = *reinterpret_cast<unsigned int*>(&_data[at]);
			at += sizeof(unsigned int);

			for (unsigned int i = 0; i < size; ++i, at += length)
				diff.second.emplace_back(new pItemNew(&_data[at], length), PacketFactory::FreePacket);
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_CONTAINER, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pContainerUpdate(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_CONTAINER, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pContainerUpdate&>(*packet).access(forward<Args&>(args)...);
	}
};

/* ************************************** */

#pragma pack(push, 1)
struct _pActorValue
{
	bool base;
	unsigned char index;
	double value;
};

struct _pActorState
{
	unsigned char moving;
	unsigned char movingxy;
	unsigned char weapon;
	bool alerted;
	bool sneaking;
};

struct _pActorDead
{
	bool dead;
	unsigned short limbs;
	signed char cause;
};

struct _pActorFireweapon
{
	unsigned int weapon;
};
#pragma pack(pop)

class pActorValue : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pActorValue _data;

		pActorValue(NetworkID id, bool base, unsigned char index, double value) : pObjectUpdateDefault(pTypes::ID_ACTOR_UPDATE, pTypes::ID_UPDATE_VALUE, id)
		{
			_data.base = base;
			_data.index = index;
			_data.value = value;
			construct(&_data, sizeof(_data));
		}
		pActorValue(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, bool& base, unsigned char& index, double& value) const
		{
			id = this->id;
			base = _data.base;
			index = _data.index;
			value = _data.value;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_VALUE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorValue(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_VALUE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pActorValue&>(*packet).access(forward<Args&>(args)...);
	}
};

class pActorState : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pActorState _data;

		pActorState(NetworkID id, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking) : pObjectUpdateDefault(pTypes::ID_ACTOR_UPDATE, pTypes::ID_UPDATE_STATE, id)
		{
			_data.moving = moving;
			_data.movingxy = movingxy;
			_data.weapon = weapon;
			_data.alerted = alerted;
			_data.sneaking = sneaking;
			construct(&_data, sizeof(_data));
		}
		pActorState(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, unsigned char& moving, unsigned char& movingxy, unsigned char& weapon, bool& alerted, bool& sneaking) const
		{
			id = this->id;
			moving = _data.moving;
			movingxy = _data.movingxy;
			weapon = _data.weapon;
			alerted = _data.alerted;
			sneaking = _data.sneaking;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_STATE, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorState(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_STATE, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pActorState&>(*packet).access(forward<Args&>(args)...);
	}
};

class pActorDead : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pActorDead _data;

		pActorDead(NetworkID id, bool dead, unsigned short limbs, signed char cause) : pObjectUpdateDefault(pTypes::ID_ACTOR_UPDATE, pTypes::ID_UPDATE_DEAD, id)
		{
			_data.dead = dead;
			_data.limbs = limbs;
			_data.cause = cause;
			construct(&_data, sizeof(_data));
		}
		pActorDead(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, bool& dead, unsigned short& limbs, signed char& cause) const
		{
			id = this->id;
			dead = _data.dead;
			limbs = _data.limbs;
			cause = _data.cause;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_DEAD, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorDead(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_DEAD, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pActorDead&>(*packet).access(forward<Args&>(args)...);
	}
};

class pActorFireweapon : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pActorFireweapon _data;

		pActorFireweapon(NetworkID id, unsigned int weapon) : pObjectUpdateDefault(pTypes::ID_ACTOR_UPDATE, pTypes::ID_UPDATE_FIREWEAPON, id)
		{
			_data.weapon = weapon;
			construct(&_data, sizeof(_data));
		}
		pActorFireweapon(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, unsigned int& weapon) const
		{
			id = this->id;
			weapon = _data.weapon;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_FIREWEAPON, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pActorFireweapon(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_FIREWEAPON, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pActorFireweapon&>(*packet).access(forward<Args&>(args)...);
	}
};

/* ************************************** */

#pragma pack(push, 1)
struct _pPlayerControl
{
	unsigned char control;
	unsigned char key;
};

struct _pPlayerInterior
{
	char cell[MAX_CELL_NAME];
};

struct _pPlayerExterior
{
	unsigned int baseID;
	signed int x;
	signed int y;
};
#pragma pack(pop)

class pPlayerControl : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pPlayerControl _data;

		pPlayerControl(NetworkID id, unsigned char control, unsigned char key) : pObjectUpdateDefault(pTypes::ID_PLAYER_UPDATE, pTypes::ID_UPDATE_CONTROL, id)
		{
			_data.control = control;
			_data.key = key;
			construct(&_data, sizeof(_data));
		}
		pPlayerControl(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, unsigned char& control, unsigned char& key) const
		{
			id = this->id;
			control = _data.control;
			key = _data.key;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_CONTROL, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerControl(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_CONTROL, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pPlayerControl&>(*packet).access(forward<Args&>(args)...);
	}
};

class pPlayerInterior : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pPlayerInterior _data;

		pPlayerInterior(NetworkID id, const string& cell) : pObjectUpdateDefault(pTypes::ID_PLAYER_UPDATE, pTypes::ID_UPDATE_INTERIOR, id)
		{
			strncpy(_data.cell, cell.c_str(), sizeof(_data.cell));
			construct(&_data, sizeof(_data));
		}
		pPlayerInterior(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, string& cell) const
		{
			id = this->id;
			cell.assign(_data.cell, find(_data.cell, _data.cell + sizeof(_data.cell), 0));
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_INTERIOR, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerInterior(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_INTERIOR, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pPlayerInterior&>(*packet).access(forward<Args&>(args)...);
	}
};

class pPlayerExterior : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pPlayerExterior _data;

		pPlayerExterior(NetworkID id, unsigned int baseID, signed int x, signed int y) : pObjectUpdateDefault(pTypes::ID_PLAYER_UPDATE, pTypes::ID_UPDATE_EXTERIOR, id)
		{
			_data.baseID = baseID;
			_data.x = x;
			_data.y = y;
			construct(&_data, sizeof(_data));
		}
		pPlayerExterior(const unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
		{
			deconstruct(&_data, sizeof(_data));
		}

		void access(NetworkID& id, unsigned int& baseID, signed int& x, signed int& y) const
		{
			id = this->id;
			baseID = _data.baseID;
			x = _data.x;
			y = _data.y;
		}
};

template<typename... Args>
struct PacketFactory::_Create<pTypes::ID_UPDATE_EXTERIOR, Args...> {
	inline static pPacket Create(Args&&... args) {
		return pPacket(new pPlayerExterior(forward<Args>(args)...), FreePacket);
	}
};

template<typename... Args>
struct PacketFactory::_Access<pTypes::ID_UPDATE_EXTERIOR, Args...> {
	inline static void Access(const pDefault* packet, Args&... args) {
		dynamic_cast<const pPlayerExterior&>(*packet).access(forward<Args&>(args)...);
	}
};

#endif
