/*
	Copyright (c) 2009-2010 Christopher A. Taylor.  All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.
	* Neither the name of LibCat nor the names of its contributors may be used
	  to endorse or promote products derived from this software without
	  specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

#include <cat/port/AlignedAlloc.hpp>
#include <cstdlib>
using namespace std;
using namespace cat;


#if defined(CAT_OS_WINDOWS)
#include <cat/port/WindowsInclude.hpp>
#endif


// Object for placement new
Aligned Aligned::ii;


//// Cache line size determination

static u32 _cacheline_bytes = 0;

static CAT_INLINE u32 DetermineCacheLineBytes()
{
#if defined(CAT_ASM_INTEL) && defined(CAT_ISA_X86)

	u32 cacheline = 0;

	CAT_ASM_BEGIN
		push ebx
		xor ecx, ecx
next0:	mov eax, 4
		cpuid
		test eax, 31
		jz done0
		or [cacheline], ebx
		lea ecx, [ecx+1]
		jmp next0
done0:	pop ebx
	CAT_ASM_END

	return (cacheline & 4095) + 1;

#elif defined(CAT_ASM_ATT) && defined(CAT_ISA_X86)

	u32 cacheline = 0;
	CAT_ASM_BEGIN
		"movl %%ebx, %%edx\n\t"
		"xorl %%ecx, %%ecx\n\t"
		"next: movl $4, %%eax\n\t"
		"cpuid\n\t"
		"testl $31, %%eax\n\t"
		"jz done\n\t"
		"orl %%ebx, %0\n\t"
		"leal 1(%%ecx), %%ecx\n\t"
		"jmp next\n\t"
		"done: ;"
		"movl %%edx, %%ebx\n\t"
		: "=r" (cacheline)
		: /* no inputs */
		: "cc", "eax", "ecx", "edx"
	CAT_ASM_END

	return (cacheline & 4095) + 1;

#else

	return CAT_DEFAULT_CACHE_LINE_SIZE;

#endif
}

u32 cat::GetCacheLineBytes()
{
	if (!_cacheline_bytes)
		_cacheline_bytes = DetermineCacheLineBytes();

	return _cacheline_bytes;
}


//// Small to medium -size aligned heap allocator

static CAT_INLINE u8 DetermineOffset(void *ptr)
{
#if defined(CAT_WORD_64)
	return (u8)( _cacheline_bytes - ((u32)*(u64*)&ptr & (_cacheline_bytes-1)) );
#else
	return (u8)( _cacheline_bytes - (*(u32*)&ptr & (_cacheline_bytes-1)) );
#endif
}

// Allocates memory aligned to a CPU cache-line byte boundary from the heap
void *Aligned::Acquire(u32 bytes)
{
	if (!_cacheline_bytes)
	{
		_cacheline_bytes = DetermineCacheLineBytes();
	}

    u8 *buffer = (u8*)malloc(_cacheline_bytes + bytes);
    if (!buffer) return 0;

	// Get buffer aligned address
	u8 offset = DetermineOffset(buffer);
    buffer += offset;
    buffer[-1] = offset;

    return buffer;
}

// Resizes an aligned pointer
void *Aligned::Resize(void *ptr, u32 bytes)
{
	if (!ptr) return Acquire(bytes);

	// Can assume here that cacheline bytes has been determined

	// Get buffer base address
	u8 *buffer = reinterpret_cast<u8*>( ptr );
	buffer -= buffer[-1];

	buffer = (u8*)realloc(buffer, _cacheline_bytes + bytes);
	if (!buffer) return 0;

	// Get buffer aligned address
	u8 offset = DetermineOffset(buffer);
	buffer += offset;
	buffer[-1] = offset;

	return buffer;
}

// Frees an aligned pointer
void Aligned::Release(void *ptr)
{
    if (ptr)
    {
		// Get buffer base address
        u8 *buffer = reinterpret_cast<u8*>( ptr );
        buffer -= buffer[-1];

        free(buffer);
    }
}


//// Large-size aligned heap allocator

// Allocates memory aligned to a CPU cache-line byte boundary from the heap
void *LargeAligned::Acquire(u32 bytes)
{
#if defined(CAT_OS_WINDOWS)
	return VirtualAlloc(0, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	return Aligned::Acquire(bytes);
#endif
}

// Frees an aligned pointer
void LargeAligned::Release(void *ptr)
{
	if (ptr)
	{
#if defined(CAT_OS_WINDOWS)
		VirtualFree(ptr, 0, MEM_RELEASE);
#else
		Aligned::Release(ptr);
#endif
	}
}
