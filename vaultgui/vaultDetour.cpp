#include <windows.h>
#include <iostream>

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue)) // by Matt Pietrek

/* BEGIN DETOURXS CODE BY SINNER */
#ifdef _M_IX86
#ifdef _MSC_VER// NOTE: if you're not compiling with msvc, then you'll have to set your compiler to link LDE
#pragma comment(lib, "LDE64")
#endif
extern "C" size_t __stdcall LDE( const LPVOID lpData, unsigned int size );
#else
#ifdef _MSC_VER// NOTE: if you're not compiling with msvc, then you'll have to set your compiler to link LDE
#pragma comment(lib, "LDE64x64")
#endif
extern "C" size_t __fastcall LDE( const LPVOID lpData, unsigned int size );
#endif

static const size_t relativeJmpSize = 1 + sizeof( DWORD );
static const size_t absoluteJmpSize = 2 + sizeof( DWORD ) + sizeof( DWORD_PTR );

enum JmpType :
size_t
{
	Relative = relativeJmpSize,
	Absolute = absoluteJmpSize
};

JmpType GetJmpType( const LPBYTE lpbFrom, const LPBYTE lpbTo )
{
#undef min
#undef max

	const DWORD_PTR upper = reinterpret_cast<DWORD_PTR>( std::max( lpbFrom, lpbTo ) );
	const DWORD_PTR lower = reinterpret_cast<DWORD_PTR>( std::min( lpbFrom, lpbTo ) );

	return ( upper - lower > 0x7FFFFFFF ) ? Absolute : Relative;
}

void WriteJump( const LPBYTE lpbFrom, const LPBYTE lpbTo, JmpType jmpType )
{
	if( jmpType == Absolute )
	{
		lpbFrom[0] = 0xFF;
		lpbFrom[1] = 0x25;

		// FF 25 [ptr_to_jmp(4bytes)][jmp(4bytes)]
		*reinterpret_cast<PDWORD>( lpbFrom + 2 ) = reinterpret_cast<DWORD>( lpbFrom ) + 6;
		*reinterpret_cast<PDWORD>( lpbFrom + 6 ) = reinterpret_cast<DWORD>( lpbTo );
	}

	else if( jmpType == Relative )
	{
		// E9 [to - from - jmp_size]
		lpbFrom[0] = 0xE9;
		DWORD offset = reinterpret_cast<DWORD>( lpbTo ) - reinterpret_cast<DWORD>( lpbFrom ) - relativeJmpSize;
		*reinterpret_cast<PDWORD>( lpbFrom + 1 ) = static_cast<DWORD>( offset );
	}
}

void WriteJump( const LPBYTE lpbFrom, const LPBYTE lpbTo )
{
	JmpType jmpType = GetJmpType( lpbFrom, lpbTo );
	WriteJump( lpbFrom, lpbTo, jmpType );
}
/* END DETOURXS CODE BY SINNER */

/* BEGIN FUNCLEN CODE BY DARAWK */
#define INSTR_NEAR_PREFIX 0x0F
#define INSTR_SHORTJCC_BEGIN 0x70
#define INSTR_SHORTJCC_END 0x7F
#define INSTR_NEARJCC_BEGIN 0x80	//	Near's are prefixed with a 0x0F byte
#define INSTR_NEARJCC_END 0x8F
#define INSTR_RET 0xC2
#define INSTR_RETN 0xC3
#define INSTR_RETFN 0xCA
#define INSTR_RETF 0xCB
#define INSTR_RELJCX 0xE3
#define INSTR_RELJMP 0xE9
#define INSTR_SHORTJMP 0xEB

typedef	unsigned __int8		u8;
typedef unsigned __int16	u16;
typedef unsigned __int32	u32;
typedef unsigned __int64	u64;

typedef signed __int8		s8;
typedef signed __int16		s16;
typedef signed __int32		s32;
typedef signed __int64		s64;

typedef float	f32;
typedef double	f64;

void *GetBranchAddress( u8 *instr )
{
	s32 offset = 0;

	//	This code will determine what type of branch it is, and
	//	determine the address it will branch to.
	switch( *instr )
	{
		case INSTR_SHORTJMP:
		case INSTR_RELJCX:
			offset  = ( s32 )( *( s8 * )( instr + 1 ) );
			offset += 2;
			break;

		case INSTR_RELJMP:
			offset  = *( s32 * )( instr + 1 );
			offset += 5;
			break;

		case INSTR_NEAR_PREFIX:
			if( *( instr + 1 ) >= INSTR_NEARJCC_BEGIN && *( instr + 1 ) <= INSTR_NEARJCC_END )
			{
				offset  = *( s32 * )( instr + 2 );
				offset += 5;
			}

			break;

		default:

			//	Check to see if it's in the valid range of JCC values.
			//	e.g. ja, je, jne, jb, etc..
			if( *instr >= INSTR_SHORTJCC_BEGIN && *instr <= INSTR_SHORTJCC_END )
			{
				offset  = ( s32 ) * ( ( s8 * )( instr + 1 ) );
				offset += 2;
			}

			break;
	}

	if( offset == 0 ) return NULL;

	return instr + offset;
}
/* END FUNCLEN CODE BY DARAWK */

size_t GetTrampolineLength( void* at, size_t lenOfHook )
{
	size_t cSize = 0;

	while( 1 )
	{
		void* ptr = MakePtr( void*, at, cSize );
		cSize += LDE( ptr, 0 );

		if( cSize >= lenOfHook )
			break;
	}

	return cSize;
}

void* BuildTrampoline( void* at, size_t len )
{
	void* tramp = VirtualAlloc( 0, 20, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

	if( !tramp )
		return tramp;

	DWORD ignore;
	VirtualProtect( tramp, 1024, PAGE_EXECUTE_READWRITE, &ignore ); // make executable

	memcpy( tramp, at, len ); //copy opcodes

	void* reentry = MakePtr( void*, at, len );
	void* tramp_jmp = MakePtr( void*, tramp, len );
	WriteJump( ( BYTE* )tramp_jmp, ( BYTE* )reentry );
	//*(BYTE*)tramp_jmp = 0xC3;
	return tramp;
}

void* BuildTrampolineSteam( void* at, void* steamhook )
{
	void* tramp = VirtualAlloc( 0, 20, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

	if( !tramp )
		return tramp;

	DWORD ignore;
	VirtualProtect( tramp, 1024, PAGE_EXECUTE_READWRITE, &ignore ); // make executable

	WriteJump( ( BYTE* )tramp, ( BYTE* )steamhook );
	return tramp;
}

void* DetourFunction( void* from, void* to )
{
	DWORD dwOldProt;
	VirtualProtect( from, 15, PAGE_EXECUTE_READWRITE, &dwOldProt );
	size_t tLen = GetTrampolineLength( from, 5 );
	void* tramp = BuildTrampoline( from, tLen );
	WriteJump( ( BYTE* )from, ( BYTE* )to );
	VirtualProtect( from, 15, dwOldProt, &dwOldProt );
	return tramp;
}

void* DetourForSteam( void* from, void* to )
{
	if( *( BYTE* )from == 0xE9 ) // already hooked
	{
		void* steamHook = GetBranchAddress( ( u8* )from );
		DWORD dwOldProt;
		VirtualProtect( from, 15, PAGE_EXECUTE_READWRITE, &dwOldProt );
		void* tramp = BuildTrampolineSteam( from, steamHook );
		WriteJump( ( BYTE* )from, ( BYTE* )to );
		VirtualProtect( from, 15, dwOldProt, &dwOldProt );
		return tramp;
	}

	else
	{
		return DetourFunction( from, to );
	}
}
