/*
 *  vaultscript.h
 *  Don't change anything here
 */

#ifdef __cplusplus
	#include <string>
	#include <vector>
	#include <unordered_set>
	#include <unordered_map>
	#include <cstdint>
	#define VAULTFUNCTION inline static
	#define VAULTPREFIX(name) _##name
	#define VAULTSPACE vaultmp::
	#define _CPP(expr) expr
	#define _C(expr)
#else
	#include <stdint.h>
	#define VAULTPREFIX(name) name
	#define VAULTSPACE
	#define _CPP(expr)
	#define _C(expr) expr
#endif

#ifndef __WIN32__
	#ifndef __cdecl
		#define __cdecl __attribute__((__cdecl__))
	#endif
	#define VAULTSCRIPT __attribute__ ((__visibility__("default"))) __cdecl
#else
	#define VAULTSCRIPT __declspec(dllexport) __cdecl
#endif

_CPP(
namespace vaultmp {
)
	typedef void Void;
	typedef char* RawString;
	typedef char RawChar;
	typedef const char* cRawString;
	typedef const char cRawChar;
	typedef int32_t Count;
	typedef uint32_t UCount;
	typedef double Value;
#ifndef __cplusplus
	typedef uint8_t Reason;
	typedef uint8_t Index;
	typedef uint8_t Type;
	typedef uint32_t State;
	typedef uint32_t Reference;
	typedef uint32_t Base;
	typedef uint32_t Cell;
	typedef uint32_t Interval;
	typedef uint64_t ID;
	typedef uint64_t Timer;
	typedef uint64_t Result;

	#define RawFunction(types)  Result (__cdecl*)(types)
	#define RawArray(type)		type*

	#define FALLOUT3			(0x01)
	#define NEWVEGAS			(FALLOUT3 << 1)
	#define FALLOUT_GAMES		(FALLOUT3 | NEWVEGAS)
	#define ALL_GAMES			(FALLOUT_GAMES)

	#define MAX_PLAYER_NAME		(16)
	#define MAX_PASSWORD_SIZE	(16)
	#define MAX_MESSAGE_LENGTH	(64)

	#define ID_REFERENCE		(0x01)
	#define ID_OBJECT			(ID_REFERENCE << 1)
	#define ID_ITEM				(ID_OBJECT << 1)
	#define ID_CONTAINER		(ID_ITEM << 1)
	#define ID_ACTOR			(ID_CONTAINER << 1)
	#define ID_PLAYER			(ID_ACTOR << 1)

	#define ALL_OBJECTS			(ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER)
	#define ALL_CONTAINERS		(ID_CONTAINER | ID_ACTOR | ID_PLAYER)
	#define ALL_ACTORS			(ID_ACTOR | ID_PLAYER)
#else
	enum Reason _CPP(: uint8_t);

	enum State _CPP(: bool)
	{
		True    =   _CPP(true) _C(1),
		False   =   _CPP(false) _C(0),
	};

	enum Reference _CPP(: uint32_t);
	enum Base _CPP(: uint32_t);
	enum Cell _CPP(: uint32_t);

	enum Interval _CPP(: uint32_t)
	{
		DEFAULT_PLAYER_RESPAWN  =   8000,
	};

	enum ID _CPP(: uint64_t);
	enum Timer _CPP(: uint64_t);
	enum Result _CPP(: uint64_t);

	enum _CPP(class) Index _CPP(: uint8_t)
	{
		FALLOUT3            =   0x01,
		NEWVEGAS            =   FALLOUT3 << 1,
		FALLOUT_GAMES       =   FALLOUT3 | NEWVEGAS,
		ALL_GAMES           =   FALLOUT_GAMES,

		MAX_PLAYER_NAME     =   16,
		MAX_PASSWORD_SIZE   =   16,
		MAX_MESSAGE_LENGTH  =   64,
	};

	enum _CPP(class) Type _CPP(: uint8_t)
	{
		ID_REFERENCE        =   0x01,
		ID_OBJECT           =   ID_REFERENCE << 1,
		ID_ITEM             =   ID_OBJECT << 1,
		ID_CONTAINER        =   ID_ITEM << 1,
		ID_ACTOR            =   ID_CONTAINER << 1,
		ID_PLAYER           =   ID_ACTOR << 1,

		ALL_OBJECTS         = 	(ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER),
		ALL_CONTAINERS      = 	(ID_CONTAINER | ID_ACTOR | ID_PLAYER),
		ALL_ACTORS          = 	(ID_ACTOR | ID_PLAYER),
	};

	struct _hash_ID { inline size_t operator() (const ID& x) const { return std::hash<uint64_t>()((uint64_t) x); }};

	typedef std::string String;
	typedef std::vector<ID> IDVector;
	typedef std::unordered_set<ID, _hash_ID> IDSet;

	template <typename V>
	using IDHash = std::unordered_map<ID, V, _hash_ID>;

	template <typename T>
	using RawArray = T*;

	#define RawArray(type)		RawArray<type>

	template <typename... Types>
	using Function = Result (__cdecl*)(Types...);

	#define RawFunction(types) 	Function<types>
}
#endif

_CPP(extern "C" {)
	VAULTSCRIPT VAULTSPACE Void exec();

	VAULTSCRIPT VAULTSPACE Void OnSpawn(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE Void OnCellChange(VAULTSPACE ID, VAULTSPACE Cell);
	VAULTSCRIPT VAULTSPACE Void OnContainerItemChange(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE Count, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE Void OnActorValueChange(VAULTSPACE ID, VAULTSPACE Index, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE Void OnActorBaseValueChange(VAULTSPACE ID, VAULTSPACE Index, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE Void OnActorAlert(VAULTSPACE ID, VAULTSPACE State);
	VAULTSCRIPT VAULTSPACE Void OnActorSneak(VAULTSPACE ID, VAULTSPACE State);
	VAULTSCRIPT VAULTSPACE Void OnActorDeath(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE Void OnActorEquipItem(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE Void OnActorUnequipItem(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE Void OnPlayerDisconnect(VAULTSPACE ID, VAULTSPACE Reason);
	VAULTSCRIPT VAULTSPACE Base OnPlayerRequestGame(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State OnClientAuthenticate(VAULTSPACE cRawString, VAULTSPACE cRawString);

	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(timestamp))();
	VAULTSCRIPT VAULTSPACE Timer (*VAULTPREFIX(CreateTimer))(VAULTSPACE RawFunction(), VAULTSPACE Interval);
	VAULTSCRIPT VAULTSPACE Timer (*VAULTPREFIX(CreateTimerEx))(VAULTSPACE RawFunction(), VAULTSPACE Interval, VAULTSPACE cRawString, ...);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(KillTimer))(VAULTSPACE Timer);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(MakePublic))(VAULTSPACE RawFunction(), VAULTSPACE cRawString, VAULTSPACE cRawString);
	VAULTSCRIPT VAULTSPACE Result (*VAULTPREFIX(CallPublic))(VAULTSPACE cRawString, ...);

	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(SetServerName))(VAULTSPACE cRawString);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(SetServerMap))(VAULTSPACE cRawString);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(SetServerRule))(VAULTSPACE cRawString, VAULTSPACE cRawString);
	VAULTSCRIPT VAULTSPACE Index (*VAULTPREFIX(GetGameCode))();
	VAULTSCRIPT VAULTSPACE UCount (*VAULTPREFIX(GetMaximumPlayers))();
	VAULTSCRIPT VAULTSPACE UCount (*VAULTPREFIX(GetCurrentPlayers))();

	VAULTSCRIPT VAULTSPACE cRawString (*VAULTPREFIX(ValueToString))(VAULTSPACE Index);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTPREFIX(AxisToString))(VAULTSPACE Index);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTPREFIX(AnimToString))(VAULTSPACE Index);

	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(UIMessage))(VAULTSPACE ID, VAULTSPACE cRawString);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(SetRespawn))(VAULTSPACE Interval);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsValid))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsObject))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsItem))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsContainer))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsActor))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsPlayer))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE Type (*VAULTPREFIX(GetType))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTPREFIX(GetCount))(VAULTSPACE Type);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTPREFIX(GetList))(VAULTSPACE Type, VAULTSPACE RawArray(VAULTSPACE ID)*);

	VAULTSCRIPT VAULTSPACE Reference (*VAULTPREFIX(GetReference))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE Base (*VAULTPREFIX(GetBase))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE cRawString (*VAULTPREFIX(GetName))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(GetPos))(VAULTSPACE ID, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(GetAngle))(VAULTSPACE ID, VAULTSPACE Value*, VAULTSPACE Value*, VAULTSPACE Value*);
	VAULTSCRIPT VAULTSPACE Cell (*VAULTPREFIX(GetCell))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsNearPoint))(VAULTSPACE ID, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTPREFIX(GetContainerItemCount))(VAULTSPACE ID, VAULTSPACE Base);
	VAULTSCRIPT VAULTSPACE Value (*VAULTPREFIX(GetActorValue))(VAULTSPACE ID, VAULTSPACE Index);
	VAULTSCRIPT VAULTSPACE Value (*VAULTPREFIX(GetActorBaseValue))(VAULTSPACE ID, VAULTSPACE Index);
	VAULTSCRIPT VAULTSPACE Index (*VAULTPREFIX(GetActorMovingAnimation))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(GetActorAlerted))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(GetActorSneaking))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(GetActorDead))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(IsActorJumping))(VAULTSPACE ID);

	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(AddItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE UCount, VAULTSPACE Value, VAULTSPACE State);
	VAULTSCRIPT VAULTSPACE UCount (*VAULTPREFIX(RemoveItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE UCount, VAULTSPACE State);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(RemoveAllItems))(VAULTSPACE ID);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(SetActorValue))(VAULTSPACE ID, VAULTSPACE Index, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(SetActorBaseValue))(VAULTSPACE ID, VAULTSPACE Index, VAULTSPACE Value);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(EquipItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE State, VAULTSPACE State);
	VAULTSCRIPT VAULTSPACE State (*VAULTPREFIX(UnequipItem))(VAULTSPACE ID, VAULTSPACE Base, VAULTSPACE State, VAULTSPACE State);
	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(KillActor))(VAULTSPACE ID);

	VAULTSCRIPT VAULTSPACE Void (*VAULTPREFIX(SetPlayerRespawn))(VAULTSPACE ID, VAULTSPACE Interval);
_CPP(})

#ifdef __cplusplus
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
	VAULTFUNCTION Timer CreateTimer(Function<> function, Interval interval) { return _CreateTimer(function, interval); }

	template<typename... Types>
	VAULTFUNCTION Timer CreateTimer(Result (*function)(Types...), Interval interval, Types... values)
	{
		cRawString types = TypeString<Types...>::value;
		return _CreateTimerEx(reinterpret_cast<Function<>>(function), interval, types, values...);
	}

	VAULTFUNCTION Void KillTimer(Timer timer = (Timer) 0) { _KillTimer(timer); }

	template<typename... Types>
	VAULTFUNCTION Void MakePublic(Result (*function)(Types...), String name)
	{
		cRawString types = TypeString<Types...>::value;
		_MakePublic(reinterpret_cast<Function<>>(function), name.c_str(), types);
	}

	template<typename... Types>
	VAULTFUNCTION Result CallPublic(String name, Types... values)
	{
		return _CallPublic(name.c_str(), values...);
	}

	VAULTFUNCTION Void SetServerName(String name) { _SetServerName(name.c_str()); }
	VAULTFUNCTION Void SetServerMap(String map) { _SetServerMap(map.c_str()); }
	VAULTFUNCTION Void SetServerRule(String key, String value) { _SetServerRule(key.c_str(), value.c_str()); }
	VAULTFUNCTION Index GetGameCode() { return _GetGameCode(); }
	VAULTFUNCTION UCount GetMaximumPlayers() { return _GetMaximumPlayers(); }
	VAULTFUNCTION UCount GetCurrentPlayers() { return _GetCurrentPlayers(); }

	VAULTFUNCTION String ValueToString(Index index) { return String(_ValueToString(index)); }
	VAULTFUNCTION String AxisToString(Index index) { return String(_AxisToString(index)); }
	VAULTFUNCTION String AnimToString(Index index) { return String(_AnimToString(index)); }

}
#endif

