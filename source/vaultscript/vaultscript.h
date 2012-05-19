/*
 *  vaultscript.h
 *  Don't change anything here
 */

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

#ifndef __WIN32__
#ifndef __cdecl
#define __cdecl __attribute__((__cdecl__))
#endif
#define VAULTSCRIPT __attribute__ ((__visibility__("default"))) __cdecl
#else
#define VAULTSCRIPT __declspec(dllexport) __cdecl
#endif

#define VAULTFUNCTION inline static

namespace vaultmp
{
	enum Reason : uint8_t;

	enum State : bool
	{
		True    =   true,
		False   =   false,
	};

	enum Reference : uint32_t;
	enum Base : uint32_t;
	enum Cell : uint32_t;

	typedef uint32_t UCount;
	typedef int32_t Count;

	enum Interval : uint32_t
	{
		DEFAULT_PLAYER_RESPAWN  =   8000,
	};

	enum ID : uint64_t;
	enum Timer : uint64_t;
	enum Result : uint64_t;

	struct _hash_ID { inline size_t operator() (const ID& x) const { return std::hash<uint64_t>()((uint64_t) x); }};

	typedef void Void;
	typedef double Value;
	typedef std::string String;
	typedef char* RawString;
	typedef char RawChar;
	typedef const char* cRawString;
	typedef const char cRawChar;
	typedef std::vector<ID> IDVector;
	typedef std::unordered_set<ID, _hash_ID> IDSet;

	template <typename V>
	using IDHash = std::unordered_map<ID, V, _hash_ID>;

	template <typename T>
	using RawArray = T*;

	template <typename... Types>
	using Function = Result (__cdecl*)(Types...);

	enum class Index : uint8_t
	{
		FALLOUT3            =   0x01,
		NEWVEGAS            =   FALLOUT3 << 1,
		FALLOUT_GAMES       =   FALLOUT3 | NEWVEGAS,
		ALL_GAMES           =   FALLOUT_GAMES,

		MAX_PLAYER_NAME     =   16,
		MAX_PASSWORD_SIZE   =   16,
		MAX_MESSAGE_LENGTH  =   64,
	};

	enum class Type : uint8_t
	{
		ID_REFERENCE        =   0x01,
		ID_OBJECT           =   ID_REFERENCE << 1,
		ID_ITEM             =   ID_OBJECT << 1,
		ID_CONTAINER        =   ID_ITEM << 1,
		ID_ACTOR            =   ID_CONTAINER << 1,
		ID_PLAYER           =   ID_ACTOR << 1,

		ALL_OBJECTS         = (ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER),
		ALL_CONTAINERS      = (ID_CONTAINER | ID_ACTOR | ID_PLAYER),
		ALL_ACTORS          = (ID_ACTOR | ID_PLAYER),
	};
};

extern "C" {
	VAULTSCRIPT vaultmp::Void exec();

	VAULTSCRIPT vaultmp::Void OnSpawn(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void OnCellChange(vaultmp::ID, vaultmp::Cell);
	VAULTSCRIPT vaultmp::Void OnContainerItemChange(vaultmp::ID, vaultmp::Base, vaultmp::Count, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorValueChange(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorBaseValueChange(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorAlert(vaultmp::ID, vaultmp::State);
	VAULTSCRIPT vaultmp::Void OnActorSneak(vaultmp::ID, vaultmp::State);
	VAULTSCRIPT vaultmp::Void OnActorDeath(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void OnActorEquipItem(vaultmp::ID, vaultmp::Base, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorUnequipItem(vaultmp::ID, vaultmp::Base, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnPlayerDisconnect(vaultmp::ID, vaultmp::Reason);
	VAULTSCRIPT vaultmp::Base OnPlayerRequestGame(vaultmp::ID);
	VAULTSCRIPT vaultmp::State OnClientAuthenticate(vaultmp::cRawString, vaultmp::cRawString);

	VAULTSCRIPT vaultmp::Void (*_timestamp)();
	VAULTSCRIPT vaultmp::Timer (*_CreateTimer)(vaultmp::Function<>, vaultmp::Interval);
	VAULTSCRIPT vaultmp::Timer (*_CreateTimerEx)(vaultmp::Function<>, vaultmp::Interval, vaultmp::cRawString, ...);
	VAULTSCRIPT vaultmp::Void (*_KillTimer)(vaultmp::Timer);
	VAULTSCRIPT vaultmp::Void (*_MakePublic)(vaultmp::Function<>, vaultmp::cRawString, vaultmp::cRawString);
	VAULTSCRIPT vaultmp::Result (*_CallPublic)(vaultmp::cRawString, ...);

	VAULTSCRIPT vaultmp::Void (*_SetServerName)(vaultmp::cRawString);
	VAULTSCRIPT vaultmp::Void (*_SetServerMap)(vaultmp::cRawString);
	VAULTSCRIPT vaultmp::Void (*_SetServerRule)(vaultmp::cRawString, vaultmp::cRawString);
	VAULTSCRIPT vaultmp::Index (*_GetGameCode)();
	VAULTSCRIPT vaultmp::UCount (*_GetMaximumPlayers)();
	VAULTSCRIPT vaultmp::UCount (*_GetCurrentPlayers)();

	VAULTSCRIPT vaultmp::cRawString (*_ValueToString)(vaultmp::Index);
	VAULTSCRIPT vaultmp::cRawString (*_AxisToString)(vaultmp::Index);
	VAULTSCRIPT vaultmp::cRawString (*_AnimToString)(vaultmp::Index);

	VAULTSCRIPT vaultmp::State (*_UIMessage)(vaultmp::ID, vaultmp::cRawString);
	VAULTSCRIPT vaultmp::Void (*_SetRespawn)(vaultmp::Interval);
	VAULTSCRIPT vaultmp::State (*_IsValid)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_IsObject)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_IsItem)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_IsContainer)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_IsActor)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_IsPlayer)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Type (*_GetType)(vaultmp::ID);
	VAULTSCRIPT vaultmp::UCount (*_GetCount)(vaultmp::Type);
	VAULTSCRIPT vaultmp::UCount (*_GetList)(vaultmp::Type, vaultmp::RawArray<vaultmp::ID>*);

	VAULTSCRIPT vaultmp::Reference (*_GetReference)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Base (*_GetBase)(vaultmp::ID);
	VAULTSCRIPT vaultmp::cRawString (*_GetName)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void (*_GetPos)(vaultmp::ID, vaultmp::Value&, vaultmp::Value&, vaultmp::Value&);
	VAULTSCRIPT vaultmp::Void (*_GetAngle)(vaultmp::ID, vaultmp::Value&, vaultmp::Value&, vaultmp::Value&);
	VAULTSCRIPT vaultmp::Cell (*_GetCell)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_IsNearPoint)(vaultmp::ID, vaultmp::Value, vaultmp::Value, vaultmp::Value, vaultmp::Value);
	VAULTSCRIPT vaultmp::UCount (*_GetContainerItemCount)(vaultmp::ID, vaultmp::Base);
	VAULTSCRIPT vaultmp::Value (*_GetActorValue)(vaultmp::ID, vaultmp::Index);
	VAULTSCRIPT vaultmp::Value (*_GetActorBaseValue)(vaultmp::ID, vaultmp::Index);
	VAULTSCRIPT vaultmp::Index (*_GetActorMovingAnimation)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_GetActorAlerted)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_GetActorSneaking)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_GetActorDead)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*_IsActorJumping)(vaultmp::ID);

	VAULTSCRIPT vaultmp::Void (*_AddItem)(vaultmp::ID, vaultmp::Base, vaultmp::UCount, vaultmp::Value, vaultmp::State);
	VAULTSCRIPT vaultmp::UCount (*_RemoveItem)(vaultmp::ID, vaultmp::Base, vaultmp::UCount, vaultmp::State);
	VAULTSCRIPT vaultmp::Void (*_RemoveAllItems)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void (*_SetActorValue)(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void (*_SetActorBaseValue)(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::State (*_EquipItem)(vaultmp::ID, vaultmp::Base, vaultmp::State, vaultmp::State);
	VAULTSCRIPT vaultmp::State (*_UnequipItem)(vaultmp::ID, vaultmp::Base, vaultmp::State, vaultmp::State);
	VAULTSCRIPT vaultmp::Void (*_KillActor)(vaultmp::ID);

	VAULTSCRIPT vaultmp::Void (*_SetPlayerRespawn)(vaultmp::ID, vaultmp::Interval);

}

namespace vaultmp
{
	template<typename T, size_t t>
	struct TypeChar { static_assert(!t, "Unsupported type in variadic type list"); };

	template<typename T>
	struct TypeChar<T*, sizeof(void*)> { enum { value = 'p' }; };

	template<typename T>
	struct TypeChar<T, sizeof(uint8_t)> { enum { value = 'i' }; };

	template<typename T>
	struct TypeChar<T, sizeof(uint16_t)> { enum { value = 'i' }; };

	template<typename T>
	struct TypeChar<T, sizeof(uint32_t)> { enum { value = 'i' }; };

	template<typename T>
	struct TypeChar<T, sizeof(uint64_t)> { enum { value = 'l' }; };

	template<>
	struct TypeChar<Value, sizeof(Value)> { enum { value = 'f' }; };

	template<>
	struct TypeChar<cRawString, sizeof(cRawString)> { enum { value = 's' }; };

	template<>
	struct TypeChar<RawString, sizeof(RawString)> { enum { value = 's' }; };

	template<typename... Types>
	struct TypeString {
		static cRawChar value[sizeof...(Types) + 1];
	};

	template<typename... Types>
	cRawChar TypeString<Types...>::value[sizeof...(Types) + 1] = {
		TypeChar<typeof(Types), sizeof(Types)>::value...
	};

	VAULTFUNCTION Void timestamp() { _timestamp(); }
	VAULTFUNCTION Timer CreateTimer(Function<> F, Interval I) { return _CreateTimer(F, I); }

	template<typename... Types>
	VAULTFUNCTION Timer CreateTimer(Result (*F)(Types...), Interval I, Types... t)
	{
		cRawString w = TypeString<Types...>::value;
		return _CreateTimerEx(reinterpret_cast<Function<>>(F), I, w, t...);
	}

}
