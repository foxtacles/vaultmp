#ifdef _RAKNET_SUPPORT_DL_MALLOC

/*
Default header file for malloc-2.8.x, written by Doug Lea
and released to the public domain, as explained at
http://creativecommons.org/licenses/publicdomain. 

last update: Wed May 27 14:25:17 2009  Doug Lea  (dl at gee)

This header is for ANSI C/C++ only.  You can set any of
the following #defines before including:

* If USE_DL_PREFIX is defined, it is assumed that malloc.c 
was also compiled with this option, so all routines
have names starting with "dl".

* If HAVE_USR_INCLUDE_MALLOC_H is defined, it is assumed that this
file will be #included AFTER <malloc.h>. This is needed only if
your system defines a struct mallinfo that is incompatible with the
standard one declared here.  Otherwise, you can include this file
INSTEAD of your system system <malloc.h>.  At least on ANSI, all
declarations should be compatible with system versions

* If MSPACES is defined, declarations for mspace versions are included.
*/

#ifndef MALLOC_280_H
#define MALLOC_280_H

#include "rdlmalloc-options.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>   /* for size_t */

#ifndef ONLY_MSPACES
#define ONLY_MSPACES 0     /* define to a value */
#endif  /* ONLY_MSPACES */
#ifndef NO_MALLINFO
#define NO_MALLINFO 0
#endif  /* NO_MALLINFO */


#if !ONLY_MSPACES

#ifndef USE_DL_PREFIX
#define rdlcalloc               calloc
#define rdlfree                 free
#define rdlmalloc               malloc
#define rdlmemalign             memalign
#define rdlrealloc              realloc
#define rdlvalloc               valloc
#define rdlpvalloc              pvalloc
#define rdlmallinfo             mallinfo
#define rdlmallopt              mallopt
#define rdlmalloc_trim          malloc_trim
#define rdlmalloc_stats         malloc_stats
#define rdlmalloc_usable_size   malloc_usable_size
#define rdlmalloc_footprint     malloc_footprint
#define rdlindependent_calloc   independent_calloc
#define rdlindependent_comalloc independent_comalloc
#endif /* USE_DL_PREFIX */
#if !NO_MALLINFO 
#ifndef HAVE_USR_INCLUDE_MALLOC_H
#ifndef _MALLOC_H
#ifndef MALLINFO_FIELD_TYPE
#define MALLINFO_FIELD_TYPE size_t
#endif /* MALLINFO_FIELD_TYPE */
#ifndef STRUCT_MALLINFO_DECLARED
#define STRUCT_MALLINFO_DECLARED 1
	struct mallinfo {
		MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system */
		MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
		MALLINFO_FIELD_TYPE smblks;   /* always 0 */
		MALLINFO_FIELD_TYPE hblks;    /* always 0 */
		MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
		MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
		MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
		MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
		MALLINFO_FIELD_TYPE fordblks; /* total free space */
		MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
	};
#endif /* STRUCT_MALLINFO_DECLARED */
#endif  /* _MALLOC_H */
#endif  /* HAVE_USR_INCLUDE_MALLOC_H */
#endif  /* !NO_MALLINFO */

	/*
	malloc(size_t n)
	Returns a pointer to a newly allocated chunk of at least n bytes, or
	null if no space is available, in which case errno is set to ENOMEM
	on ANSI C systems.

	If n is zero, malloc returns a minimum-sized chunk. (The minimum
	size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
	systems.)  Note that size_t is an unsigned type, so calls with
	arguments that would be negative if signed are interpreted as
	requests for huge amounts of space, which will often fail. The
	maximum supported value of n differs across systems, but is in all
	cases less than the maximum representable value of a size_t.
	*/
	void* rdlmalloc(size_t);

	/*
	free(void* p)
	Releases the chunk of memory pointed to by p, that had been previously
	allocated using malloc or a related routine such as realloc.
	It has no effect if p is null. If p was not malloced or already
	freed, free(p) will by default cuase the current program to abort.
	*/
	void  rdlfree(void*);

	/*
	calloc(size_t n_elements, size_t element_size);
	Returns a pointer to n_elements * element_size bytes, with all locations
	set to zero.
	*/
	void* rdlcalloc(size_t, size_t);

	/*
	realloc(void* p, size_t n)
	Returns a pointer to a chunk of size n that contains the same data
	as does chunk p up to the minimum of (n, p's size) bytes, or null
	if no space is available.

	The returned pointer may or may not be the same as p. The algorithm
	prefers extending p in most cases when possible, otherwise it
	employs the equivalent of a malloc-copy-free sequence.

	If p is null, realloc is equivalent to malloc.

	If space is not available, realloc returns null, errno is set (if on
	ANSI) and p is NOT freed.

	if n is for fewer bytes than already held by p, the newly unused
	space is lopped off and freed if possible.  realloc with a size
	argument of zero (re)allocates a minimum-sized chunk.

	The old unix realloc convention of allowing the last-free'd chunk
	to be used as an argument to realloc is not supported.
	*/

	void* rdlrealloc(void*, size_t);

	/*
	memalign(size_t alignment, size_t n);
	Returns a pointer to a newly allocated chunk of n bytes, aligned
	in accord with the alignment argument.

	The alignment argument should be a power of two. If the argument is
	not a power of two, the nearest greater power is used.
	8-byte alignment is guaranteed by normal malloc calls, so don't
	bother calling memalign with an argument of 8 or less.

	Overreliance on memalign is a sure way to fragment space.
	*/
	void* rdlmemalign(size_t, size_t);

	/*
	valloc(size_t n);
	Equivalent to memalign(pagesize, n), where pagesize is the page
	size of the system. If the pagesize is unknown, 4096 is used.
	*/
	void* rdlvalloc(size_t);

	/*
	mallopt(int parameter_number, int parameter_value)
	Sets tunable parameters The format is to provide a
	(parameter-number, parameter-value) pair.  mallopt then sets the
	corresponding parameter to the argument value if it can (i.e., so
	long as the value is meaningful), and returns 1 if successful else
	0.  SVID/XPG/ANSI defines four standard param numbers for mallopt,
	normally defined in malloc.h.  None of these are use in this malloc,
	so setting them has no effect. But this malloc also supports other
	options in mallopt:

	Symbol            param #  default    allowed param values
	M_TRIM_THRESHOLD     -1   2*1024*1024   any   (-1U disables trimming)
	M_GRANULARITY        -2     page size   any power of 2 >= page size
	M_MMAP_THRESHOLD     -3      256*1024   any   (or 0 if no MMAP support)
	*/
	int rdlmallopt(int, int);

#define M_TRIM_THRESHOLD     (-1)
#define M_GRANULARITY        (-2)
#define M_MMAP_THRESHOLD     (-3)


	/*
	malloc_footprint();
	Returns the number of bytes obtained from the system.  The total
	number of bytes allocated by malloc, realloc etc., is less than this
	value. Unlike mallinfo, this function returns only a precomputed
	result, so can be called frequently to monitor memory consumption.
	Even if locks are otherwise defined, this function does not use them,
	so results might not be up to date.
	*/
	size_t rdlmalloc_footprint();

#if !NO_MALLINFO
	/*
	mallinfo()
	Returns (by copy) a struct containing various summary statistics:

	arena:     current total non-mmapped bytes allocated from system
	ordblks:   the number of free chunks
	smblks:    always zero.
	hblks:     current number of mmapped regions
	hblkhd:    total bytes held in mmapped regions
	usmblks:   the maximum total allocated space. This will be greater
	than current total if trimming has occurred.
	fsmblks:   always zero
	uordblks:  current total allocated space (normal or mmapped)
	fordblks:  total free space
	keepcost:  the maximum number of bytes that could ideally be released
	back to system via malloc_trim. ("ideally" means that
	it ignores page restrictions etc.)

	Because these fields are ints, but internal bookkeeping may
	be kept as longs, the reported values may wrap around zero and
	thus be inaccurate.
	*/

	struct mallinfo rdlmallinfo(void);
#endif  /* NO_MALLINFO */

	/*
	independent_calloc(size_t n_elements, size_t element_size, void* chunks[]);

	independent_calloc is similar to calloc, but instead of returning a
	single cleared space, it returns an array of pointers to n_elements
	independent elements that can hold contents of size elem_size, each
	of which starts out cleared, and can be independently freed,
	realloc'ed etc. The elements are guaranteed to be adjacently
	allocated (this is not guaranteed to occur with multiple callocs or
	mallocs), which may also improve cache locality in some
	applications.

	The "chunks" argument is optional (i.e., may be null, which is
	probably the most typical usage). If it is null, the returned array
	is itself dynamically allocated and should also be freed when it is
	no longer needed. Otherwise, the chunks array must be of at least
	n_elements in length. It is filled in with the pointers to the
	chunks.

	In either case, independent_calloc returns this pointer array, or
	null if the allocation failed.  If n_elements is zero and "chunks"
	is null, it returns a chunk representing an array with zero elements
	(which should be freed if not wanted).

	Each element must be individually freed when it is no longer
	needed. If you'd like to instead be able to free all at once, you
	should instead use regular calloc and assign pointers into this
	space to represent elements.  (In this case though, you cannot
	independently free elements.)

	independent_calloc simplifies and speeds up implementations of many
	kinds of pools.  It may also be useful when constructing large data
	structures that initially have a fixed number of fixed-sized nodes,
	but the number is not known at compile time, and some of the nodes
	may later need to be freed. For example:

	struct Node { int item; struct Node* next; };

	struct Node* build_list() {
	struct Node** pool;
	int n = read_number_of_nodes_needed();
	if (n <= 0) return 0;
	pool = (struct Node**)(independent_calloc(n, sizeof(struct Node), 0);
	if (pool == 0) die();
	// organize into a linked list...
	struct Node* first = pool[0];
	for (i = 0; i < n-1; ++i)
	pool[i]->next = pool[i+1];
	free(pool);     // Can now free the array (or not, if it is needed later)
	return first;
	}
	*/
	void** rdlindependent_calloc(size_t, size_t, void**);

	/*
	independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);

	independent_comalloc allocates, all at once, a set of n_elements
	chunks with sizes indicated in the "sizes" array.    It returns
	an array of pointers to these elements, each of which can be
	independently freed, realloc'ed etc. The elements are guaranteed to
	be adjacently allocated (this is not guaranteed to occur with
	multiple callocs or mallocs), which may also improve cache locality
	in some applications.

	The "chunks" argument is optional (i.e., may be null). If it is null
	the returned array is itself dynamically allocated and should also
	be freed when it is no longer needed. Otherwise, the chunks array
	must be of at least n_elements in length. It is filled in with the
	pointers to the chunks.

	In either case, independent_comalloc returns this pointer array, or
	null if the allocation failed.  If n_elements is zero and chunks is
	null, it returns a chunk representing an array with zero elements
	(which should be freed if not wanted).

	Each element must be individually freed when it is no longer
	needed. If you'd like to instead be able to free all at once, you
	should instead use a single regular malloc, and assign pointers at
	particular offsets in the aggregate space. (In this case though, you
	cannot independently free elements.)

	independent_comallac differs from independent_calloc in that each
	element may have a different size, and also that it does not
	automatically clear elements.

	independent_comalloc can be used to speed up allocation in cases
	where several structs or objects must always be allocated at the
	same time.  For example:

	struct Head { ... }
	struct Foot { ... }

	void send_message(char* msg) {
	int msglen = strlen(msg);
	size_t sizes[3] = { sizeof(struct Head), msglen, sizeof(struct Foot) };
	void* chunks[3];
	if (independent_comalloc(3, sizes, chunks) == 0)
	die();
	struct Head* head = (struct Head*)(chunks[0]);
	char*        body = (char*)(chunks[1]);
	struct Foot* foot = (struct Foot*)(chunks[2]);
	// ...
	}

	In general though, independent_comalloc is worth using only for
	larger values of n_elements. For small values, you probably won't
	detect enough difference from series of malloc calls to bother.

	Overuse of independent_comalloc can increase overall memory usage,
	since it cannot reuse existing noncontiguous small chunks that
	might be available for some of the elements.
	*/
	void** rdlindependent_comalloc(size_t, size_t*, void**);


	/*
	pvalloc(size_t n);
	Equivalent to valloc(minimum-page-that-holds(n)), that is,
	round up n to nearest pagesize.
	*/
	void*  rdlpvalloc(size_t);

	/*
	malloc_trim(size_t pad);

	If possible, gives memory back to the system (via negative arguments
	to sbrk) if there is unused memory at the `high' end of the malloc
	pool or in unused MMAP segments. You can call this after freeing
	large blocks of memory to potentially reduce the system-level memory
	requirements of a program. However, it cannot guarantee to reduce
	memory. Under some allocation patterns, some large free blocks of
	memory will be locked between two used chunks, so they cannot be
	given back to the system.

	The `pad' argument to malloc_trim represents the amount of free
	trailing space to leave untrimmed. If this argument is zero, only
	the minimum amount of memory to maintain internal data structures
	will be left. Non-zero arguments can be supplied to maintain enough
	trailing space to service future expected allocations without having
	to re-obtain memory from the system.

	Malloc_trim returns 1 if it actually released any memory, else 0.
	*/
	int  rdlmalloc_trim(size_t);

	/*
	malloc_stats();
	Prints on stderr the amount of space obtained from the system (both
	via sbrk and mmap), the maximum amount (which may be more than
	current if malloc_trim and/or munmap got called), and the current
	number of bytes allocated via malloc (or realloc, etc) but not yet
	freed. Note that this is the number of bytes allocated, not the
	number requested. It will be larger than the number requested
	because of alignment and bookkeeping overhead. Because it includes
	alignment wastage as being in use, this figure may be greater than
	zero even when no user-level chunks are allocated.

	The reported current and maximum system memory can be inaccurate if
	a program makes other calls to system memory allocation functions
	(normally sbrk) outside of malloc.

	malloc_stats prints only the most commonly interesting statistics.
	More information can be obtained by calling mallinfo.
	*/
	void  rdlmalloc_stats();

#endif /* !ONLY_MSPACES */

	/*
	malloc_usable_size(void* p);

	Returns the number of bytes you can actually use in
	an allocated chunk, which may be more than you requested (although
	often not) due to alignment and minimum size constraints.
	You can use this many bytes without worrying about
	overwriting other allocated objects. This is not a particularly great
	programming practice. malloc_usable_size can be more useful in
	debugging and assertions, for example:

	p = malloc(n);
	assert(malloc_usable_size(p) >= 256);
	*/
	size_t rdlmalloc_usable_size(void*);


#if MSPACES

	/*
	mspace is an opaque type representing an independent
	region of space that supports rak_mspace_malloc, etc.
	*/
	typedef void* mspace;

	/*
	rak_create_mspace creates and returns a new independent space with the
	given initial capacity, or, if 0, the default granularity size.  It
	returns null if there is no system memory available to create the
	space.  If argument locked is non-zero, the space uses a separate
	lock to control access. The capacity of the space will grow
	dynamically as needed to service rak_mspace_malloc requests.  You can
	control the sizes of incremental increases of this space by
	compiling with a different DEFAULT_GRANULARITY or dynamically
	setting with mallopt(M_GRANULARITY, value).
	*/
	mspace rak_create_mspace(size_t capacity, int locked);

	/*
	rak_destroy_mspace destroys the given space, and attempts to return all
	of its memory back to the system, returning the total number of
	bytes freed. After destruction, the results of access to all memory
	used by the space become undefined.
	*/
	size_t rak_destroy_mspace(mspace msp);

	/*
	rak_create_mspace_with_base uses the memory supplied as the initial base
	of a new mspace. Part (less than 128*sizeof(size_t) bytes) of this
	space is used for bookkeeping, so the capacity must be at least this
	large. (Otherwise 0 is returned.) When this initial space is
	exhausted, additional memory will be obtained from the system.
	Destroying this space will deallocate all additionally allocated
	space (if possible) but not the initial base.
	*/
	mspace rak_create_mspace_with_base(void* base, size_t capacity, int locked);

	/*
	rak_mspace_track_large_chunks controls whether requests for large chunks
	are allocated in their own untracked mmapped regions, separate from
	others in this mspace. By default large chunks are not tracked,
	which reduces fragmentation. However, such chunks are not
	necessarily released to the system upon rak_destroy_mspace.  Enabling
	tracking by setting to true may increase fragmentation, but avoids
	leakage when relying on rak_destroy_mspace to release all memory
	allocated using this space.  The function returns the previous
	setting.
	*/
	int rak_mspace_track_large_chunks(mspace msp, int enable);

	/*
	rak_mspace_malloc behaves as malloc, but operates within
	the given space.
	*/
	void* rak_mspace_malloc(mspace msp, size_t bytes);

	/*
	rak_mspace_free behaves as free, but operates within
	the given space.

	If compiled with FOOTERS==1, rak_mspace_free is not actually needed.
	free may be called instead of rak_mspace_free because freed chunks from
	any space are handled by their originating spaces.
	*/
	void rak_mspace_free(mspace msp, void* mem);

	/*
	rak_mspace_realloc behaves as realloc, but operates within
	the given space.

	If compiled with FOOTERS==1, rak_mspace_realloc is not actually
	needed.  realloc may be called instead of rak_mspace_realloc because
	realloced chunks from any space are handled by their originating
	spaces.
	*/
	void* rak_mspace_realloc(mspace msp, void* mem, size_t newsize);

	/*
	rak_mspace_calloc behaves as calloc, but operates within
	the given space.
	*/
	void* rak_mspace_calloc(mspace msp, size_t n_elements, size_t elem_size);

	/*
	rak_mspace_memalign behaves as memalign, but operates within
	the given space.
	*/
	void* rak_mspace_memalign(mspace msp, size_t alignment, size_t bytes);

	/*
	rak_mspace_independent_calloc behaves as independent_calloc, but
	operates within the given space.
	*/
	void** rak_mspace_independent_calloc(mspace msp, size_t n_elements,
		size_t elem_size, void* chunks[]);

	/*
	rak_mspace_independent_comalloc behaves as independent_comalloc, but
	operates within the given space.
	*/
	void** rak_mspace_independent_comalloc(mspace msp, size_t n_elements,
		size_t sizes[], void* chunks[]);

	/*
	rak_mspace_footprint() returns the number of bytes obtained from the
	system for this space.
	*/
	size_t rak_mspace_footprint(mspace msp);


#if !NO_MALLINFO
	/*
	rak_mspace_mallinfo behaves as mallinfo, but reports properties of
	the given space.
	*/
	struct mallinfo rak_mspace_mallinfo(mspace msp);
#endif /* NO_MALLINFO */

	/*
	malloc_usable_size(void* p) behaves the same as malloc_usable_size;
	*/
	size_t rak_mspace_usable_size(void* mem);

	/*
	rak_mspace_malloc_stats behaves as malloc_stats, but reports
	properties of the given space.
	*/
	void rak_mspace_malloc_stats(mspace msp);

	/*
	rak_mspace_trim behaves as malloc_trim, but
	operates within the given space.
	*/
	int rak_mspace_trim(mspace msp, size_t pad);

	/*
	An alias for mallopt.
	*/
	int rak_mspace_mallopt(int, int);

#endif  /* MSPACES */

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

/*
This is a version (aka rdlmalloc) of malloc/free/realloc written by
Doug Lea and released to the public domain, as explained at
http://creativecommons.org/licenses/publicdomain.  Send questions,
comments, complaints, performance data, etc to dl@cs.oswego.edu

* Version 2.8.4 Wed May 27 09:56:23 2009  Doug Lea  (dl at gee)

Note: There may be an updated version of this malloc obtainable at
ftp://gee.cs.oswego.edu/pub/misc/malloc.c
Check before installing!

* Quickstart

This library is all in one file to simplify the most common usage:
ftp it, compile it (-O3), and link it into another program. All of
the compile-time options default to reasonable values for use on
most platforms.  You might later want to step through various
compile-time and dynamic tuning options.

For convenience, an include file for code using this malloc is at:
ftp://gee.cs.oswego.edu/pub/misc/malloc-2.8.4.h
You don't really need this .h file unless you call functions not
defined in your system include files.  The .h file contains only the
excerpts from this file needed for using this malloc on ANSI C/C++
systems, so long as you haven't changed compile-time options about
naming and tuning parameters.  If you do, then you can create your
own malloc.h that does include all settings by cutting at the point
indicated below. Note that you may already by default be using a C
library containing a malloc that is based on some version of this
malloc (for example in linux). You might still want to use the one
in this file to customize settings or to avoid overheads associated
with library versions.

* Vital statistics:

Supported pointer/size_t representation:       4 or 8 bytes
size_t MUST be an unsigned type of the same width as
pointers. (If you are using an ancient system that declares
size_t as a signed type, or need it to be a different width
than pointers, you can use a previous release of this malloc
(e.g. 2.7.2) supporting these.)

Alignment:                                     8 bytes (default)
This suffices for nearly all current machines and C compilers.
However, you can define MALLOC_ALIGNMENT to be wider than this
if necessary (up to 128bytes), at the expense of using more space.

Minimum overhead per allocated chunk:   4 or  8 bytes (if 4byte sizes)
8 or 16 bytes (if 8byte sizes)
Each malloced chunk has a hidden word of overhead holding size
and status information, and additional cross-check word
if FOOTERS is defined.

Minimum allocated size: 4-byte ptrs:  16 bytes    (including overhead)
8-byte ptrs:  32 bytes    (including overhead)

Even a request for zero bytes (i.e., malloc(0)) returns a
pointer to something of the minimum allocatable size.
The maximum overhead wastage (i.e., number of extra bytes
allocated than were requested in malloc) is less than or equal
to the minimum size, except for requests >= mmap_threshold that
are serviced via mmap(), where the worst case wastage is about
32 bytes plus the remainder from a system page (the minimal
mmap unit); typically 4096 or 8192 bytes.

Security: static-safe; optionally more or less
The "security" of malloc refers to the ability of malicious
code to accentuate the effects of errors (for example, freeing
space that is not currently malloc'ed or overwriting past the
ends of chunks) in code that calls malloc.  This malloc
guarantees not to modify any memory locations below the base of
heap, i.e., static variables, even in the presence of usage
errors.  The routines additionally detect most improper frees
and reallocs.  All this holds as long as the static bookkeeping
for malloc itself is not corrupted by some other means.  This
is only one aspect of security -- these checks do not, and
cannot, detect all possible programming errors.

If FOOTERS is defined nonzero, then each allocated chunk
carries an additional check word to verify that it was malloced
from its space.  These check words are the same within each
execution of a program using malloc, but differ across
executions, so externally crafted fake chunks cannot be
freed. This improves security by rejecting frees/reallocs that
could corrupt heap memory, in addition to the checks preventing
writes to statics that are always on.  This may further improve
security at the expense of time and space overhead.  (Note that
FOOTERS may also be worth using with MSPACES.)

By default detected errors cause the program to abort (calling
"abort()"). You can override this to instead proceed past
errors by defining PROCEED_ON_ERROR.  In this case, a bad free
has no effect, and a malloc that encounters a bad address
caused by user overwrites will ignore the bad address by
dropping pointers and indices to all known memory. This may
be appropriate for programs that should continue if at all
possible in the face of programming errors, although they may
run out of memory because dropped memory is never reclaimed.

If you don't like either of these options, you can define
CORRUPTION_ERROR_ACTION and USAGE_ERROR_ACTION to do anything
else. And if if you are sure that your program using malloc has
no errors or vulnerabilities, you can define INSECURE to 1,
which might (or might not) provide a small performance improvement.

Thread-safety: NOT thread-safe unless USE_LOCKS defined
When USE_LOCKS is defined, each public call to malloc, free,
etc is surrounded with either a pthread mutex or a win32
spinlock (depending on DL_PLATFORM_WIN32). This is not especially fast, and
can be a major bottleneck.  It is designed only to provide
minimal protection in concurrent environments, and to provide a
basis for extensions.  If you are using malloc in a concurrent
program, consider instead using nedmalloc
(http://www.nedprod.com/programs/portable/nedmalloc/) or
ptmalloc (See http://www.malloc.de), which are derived
from versions of this malloc.

System requirements: Any combination of MORECORE and/or MMAP/MUNMAP
This malloc can use unix sbrk or any emulation (invoked using
the CALL_MORECORE macro) and/or mmap/munmap or any emulation
(invoked using CALL_MMAP/CALL_MUNMAP) to get and release system
memory.  On most unix systems, it tends to work best if both
MORECORE and MMAP are enabled.  On Win32, it uses emulations
based on VirtualAlloc. It also uses common C library functions
like memset.

Compliance: I believe it is compliant with the Single Unix Specification
(See http://www.unix.org). Also SVID/XPG, ANSI C, and probably
others as well.

* Overview of algorithms

This is not the fastest, most space-conserving, most portable, or
most tunable malloc ever written. However it is among the fastest
while also being among the most space-conserving, portable and
tunable.  Consistent balance across these factors results in a good
general-purpose allocator for malloc-intensive programs.

In most ways, this malloc is a best-fit allocator. Generally, it
chooses the best-fitting existing chunk for a request, with ties
broken in approximately least-recently-used order. (This strategy
normally maintains low fragmentation.) However, for requests less
than 256bytes, it deviates from best-fit when there is not an
exactly fitting available chunk by preferring to use space adjacent
to that used for the previous small request, as well as by breaking
ties in approximately most-recently-used order. (These enhance
locality of series of small allocations.)  And for very large requests
(>= 256Kb by default), it relies on system memory mapping
facilities, if supported.  (This helps avoid carrying around and
possibly fragmenting memory used only for large chunks.)

All operations (except malloc_stats and mallinfo) have execution
times that are bounded by a constant factor of the number of bits in
a size_t, not counting any clearing in calloc or copying in realloc,
or actions surrounding MORECORE and MMAP that have times
proportional to the number of non-contiguous regions returned by
system allocation routines, which is often just 1. In real-time
applications, you can optionally suppress segment traversals using
NO_SEGMENT_TRAVERSAL, which assures bounded execution even when
system allocators return non-contiguous spaces, at the typical
expense of carrying around more memory and increased fragmentation.

The implementation is not very modular and seriously overuses
macros. Perhaps someday all C compilers will do as good a job
inlining modular code as can now be done by brute-force expansion,
but now, enough of them seem not to.

Some compilers issue a lot of warnings about code that is
dead/unreachable only on some platforms, and also about intentional
uses of negation on unsigned types. All known cases of each can be
ignored.

For a longer but out of date high-level description, see
http://gee.cs.oswego.edu/dl/html/malloc.html

* MSPACES
If MSPACES is defined, then in addition to malloc, free, etc.,
this file also defines rak_mspace_malloc, rak_mspace_free, etc. These
are versions of malloc routines that take an "mspace" argument
obtained using rak_create_mspace, to control all internal bookkeeping.
If ONLY_MSPACES is defined, only these versions are compiled.
So if you would like to use this allocator for only some allocations,
and your system malloc for others, you can compile with
ONLY_MSPACES and then do something like...
static mspace mymspace = rak_create_mspace(0,0); // for example
#define mymalloc(bytes)  rak_mspace_malloc(mymspace, bytes)

(Note: If you only need one instance of an mspace, you can instead
use "USE_DL_PREFIX" to relabel the global malloc.)

You can similarly create thread-local allocators by storing
mspaces as thread-locals. For example:
static __thread mspace tlms = 0;
void*  tlmalloc(size_t bytes) {
if (tlms == 0) tlms = rak_create_mspace(0, 0);
return rak_mspace_malloc(tlms, bytes);
}
void  tlfree(void* mem) { rak_mspace_free(tlms, mem); }

Unless FOOTERS is defined, each mspace is completely independent.
You cannot allocate from one and free to another (although
conformance is only weakly checked, so usage errors are not always
caught). If FOOTERS is defined, then each chunk carries around a tag
indicating its originating mspace, and frees are directed to their
originating spaces.

-------------------------  Compile-time options ---------------------------

Be careful in setting #define values for numerical constants of type
size_t. On some systems, literal values are not automatically extended
to size_t precision unless they are explicitly casted. You can also
use the symbolic values MAX_SIZE_T, SIZE_T_ONE, etc below.

DL_PLATFORM_WIN32                    default: defined if _WIN32 defined
Defining DL_PLATFORM_WIN32 sets up defaults for MS environment and compilers.
Otherwise defaults are for unix. Beware that there seem to be some
cases where this malloc might not be a pure drop-in replacement for
Win32 malloc: Random-looking failures from Win32 GDI API's (eg;
SetDIBits()) may be due to bugs in some video driver implementations
when pixel buffers are malloc()ed, and the region spans more than
one VirtualAlloc()ed region. Because rdlmalloc uses a small (64Kb)
default granularity, pixel buffers may straddle virtual allocation
regions more often than when using the Microsoft allocator.  You can
avoid this by using VirtualAlloc() and VirtualFree() for all pixel
buffers rather than using malloc().  If this is not possible,
recompile this malloc with a larger DEFAULT_GRANULARITY.

MALLOC_ALIGNMENT         default: (size_t)8
Controls the minimum alignment for malloc'ed chunks.  It must be a
power of two and at least 8, even on machines for which smaller
alignments would suffice. It may be defined as larger than this
though. Note however that code and data structures are optimized for
the case of 8-byte alignment.

MSPACES                  default: 0 (false)
If true, compile in support for independent allocation spaces.
This is only supported if HAVE_MMAP is true.

ONLY_MSPACES             default: 0 (false)
If true, only compile in mspace versions, not regular versions.

USE_LOCKS                default: 0 (false)
Causes each call to each public routine to be surrounded with
pthread or DL_PLATFORM_WIN32 mutex lock/unlock. (If set true, this can be
overridden on a per-mspace basis for mspace versions.) If set to a
non-zero value other than 1, locks are used, but their
implementation is left out, so lock functions must be supplied manually,
as described below.

USE_SPIN_LOCKS           default: 1 iff USE_LOCKS and on x86 using gcc or MSC
If true, uses custom spin locks for locking. This is currently
supported only for x86 platforms using gcc or recent MS compilers.
Otherwise, posix locks or win32 critical sections are used.

FOOTERS                  default: 0
If true, provide extra checking and dispatching by placing
information in the footers of allocated chunks. This adds
space and time overhead.

INSECURE                 default: 0
If true, omit checks for usage errors and heap space overwrites.

USE_DL_PREFIX            default: NOT defined
Causes compiler to prefix all public routines with the string 'dl'.
This can be useful when you only want to use this malloc in one part
of a program, using your regular system malloc elsewhere.

ABORT                    default: defined as abort()
Defines how to abort on failed checks.  On most systems, a failed
check cannot die with an "assert" or even print an informative
message, because the underlying print routines in turn call malloc,
which will fail again.  Generally, the best policy is to simply call
abort(). It's not very useful to do more than this because many
errors due to overwriting will show up as address faults (null, odd
addresses etc) rather than malloc-triggered checks, so will also
abort.  Also, most compilers know that abort() does not return, so
can better optimize code conditionally calling it.

PROCEED_ON_ERROR           default: defined as 0 (false)
Controls whether detected bad addresses cause them to bypassed
rather than aborting. If set, detected bad arguments to free and
realloc are ignored. And all bookkeeping information is zeroed out
upon a detected overwrite of freed heap space, thus losing the
ability to ever return it from malloc again, but enabling the
application to proceed. If PROCEED_ON_ERROR is defined, the
static variable malloc_corruption_error_count is compiled in
and can be examined to see if errors have occurred. This option
generates slower code than the default abort policy.

DEBUG                    default: NOT defined
The DEBUG setting is mainly intended for people trying to modify
this code or diagnose problems when porting to new platforms.
However, it may also be able to better isolate user errors than just
using runtime checks.  The assertions in the check routines spell
out in more detail the assumptions and invariants underlying the
algorithms.  The checking is fairly extensive, and will slow down
execution noticeably. Calling malloc_stats or mallinfo with DEBUG
set will attempt to check every non-mmapped allocated and free chunk
in the course of computing the summaries.

ABORT_ON_ASSERT_FAILURE   default: defined as 1 (true)
Debugging assertion failures can be nearly impossible if your
version of the assert macro causes malloc to be called, which will
lead to a cascade of further failures, blowing the runtime stack.
ABORT_ON_ASSERT_FAILURE cause assertions failures to call abort(),
which will usually make debugging easier.

MALLOC_FAILURE_ACTION     default: sets errno to ENOMEM, or no-op on win32
The action to take before "return 0" when malloc fails to be able to
return memory because there is none available.

HAVE_MORECORE             default: 1 (true) unless win32 or ONLY_MSPACES
True if this system supports sbrk or an emulation of it.

MORECORE                  default: sbrk
The name of the sbrk-style system routine to call to obtain more
memory.  See below for guidance on writing custom MORECORE
functions. The type of the argument to sbrk/MORECORE varies across
systems.  It cannot be size_t, because it supports negative
arguments, so it is normally the signed type of the same width as
size_t (sometimes declared as "intptr_t").  It doesn't much matter
though. Internally, we only call it with arguments less than half
the max value of a size_t, which should work across all reasonable
possibilities, although sometimes generating compiler warnings.

MORECORE_CONTIGUOUS       default: 1 (true) if HAVE_MORECORE
If true, take advantage of fact that consecutive calls to MORECORE
with positive arguments always return contiguous increasing
addresses.  This is true of unix sbrk. It does not hurt too much to
set it true anyway, since malloc copes with non-contiguities.
Setting it false when definitely non-contiguous saves time
and possibly wasted space it would take to discover this though.

MORECORE_CANNOT_TRIM      default: NOT defined
True if MORECORE cannot release space back to the system when given
negative arguments. This is generally necessary only if you are
using a hand-crafted MORECORE function that cannot handle negative
arguments.

NO_SEGMENT_TRAVERSAL       default: 0
If non-zero, suppresses traversals of memory segments
returned by either MORECORE or CALL_MMAP. This disables
merging of segments that are contiguous, and selectively
releasing them to the OS if unused, but bounds execution times.

HAVE_MMAP                 default: 1 (true)
True if this system supports mmap or an emulation of it.  If so, and
HAVE_MORECORE is not true, MMAP is used for all system
allocation. If set and HAVE_MORECORE is true as well, MMAP is
primarily used to directly allocate very large blocks. It is also
used as a backup strategy in cases where MORECORE fails to provide
space from system. Note: A single call to MUNMAP is assumed to be
able to unmap memory that may have be allocated using multiple calls
to MMAP, so long as they are adjacent.

HAVE_MREMAP               default: 1 on linux, else 0
If true realloc() uses mremap() to re-allocate large blocks and
extend or shrink allocation spaces.

MMAP_CLEARS               default: 1 except on WINCE.
True if mmap clears memory so calloc doesn't need to. This is true
for standard unix mmap using /dev/zero and on DL_PLATFORM_WIN32 except for WINCE.

USE_BUILTIN_FFS            default: 0 (i.e., not used)
Causes malloc to use the builtin ffs() function to compute indices.
Some compilers may recognize and intrinsify ffs to be faster than the
supplied C version. Also, the case of x86 using gcc is special-cased
to an asm instruction, so is already as fast as it can be, and so
this setting has no effect. Similarly for Win32 under recent MS compilers.
(On most x86s, the asm version is only slightly faster than the C version.)

malloc_getpagesize         default: derive from system includes, or 4096.
The system page size. To the extent possible, this malloc manages
memory from the system in page-size units.  This may be (and
usually is) a function rather than a constant. This is ignored
if DL_PLATFORM_WIN32, where page size is determined using getSystemInfo during
initialization.

USE_DEV_RANDOM             default: 0 (i.e., not used)
Causes malloc to use /dev/random to initialize secure magic seed for
stamping footers. Otherwise, the current time is used.

NO_MALLINFO                default: 0
If defined, don't compile "mallinfo". This can be a simple way
of dealing with mismatches between system declarations and
those in this file.

MALLINFO_FIELD_TYPE        default: size_t
The type of the fields in the mallinfo struct. This was originally
defined as "int" in SVID etc, but is more usefully defined as
size_t. The value is used only if  HAVE_USR_INCLUDE_MALLOC_H is not set

REALLOC_ZERO_BYTES_FREES    default: not defined
This should be set if a call to realloc with zero bytes should
be the same as a call to free. Some people think it should. Otherwise,
since this malloc returns a unique pointer for malloc(0), so does
realloc(p, 0).

LACKS_UNISTD_H, LACKS_FCNTL_H, LACKS_SYS_PARAM_H, LACKS_SYS_MMAN_H
LACKS_STRINGS_H, LACKS_STRING_H, LACKS_SYS_TYPES_H,  LACKS_ERRNO_H
LACKS_STDLIB_H                default: NOT defined unless on DL_PLATFORM_WIN32
Define these if your system does not have these header files.
You might need to manually insert some of the declarations they provide.

DEFAULT_GRANULARITY        default: page size if MORECORE_CONTIGUOUS,
system_info.dwAllocationGranularity in DL_PLATFORM_WIN32,
otherwise 64K.
Also settable using mallopt(M_GRANULARITY, x)
The unit for allocating and deallocating memory from the system.  On
most systems with contiguous MORECORE, there is no reason to
make this more than a page. However, systems with MMAP tend to
either require or encourage larger granularities.  You can increase
this value to prevent system allocation functions to be called so
often, especially if they are slow.  The value must be at least one
page and must be a power of two.  Setting to 0 causes initialization
to either page size or win32 region size.  (Note: In previous
versions of malloc, the equivalent of this option was called
"TOP_PAD")

DEFAULT_TRIM_THRESHOLD    default: 2MB
Also settable using mallopt(M_TRIM_THRESHOLD, x)
The maximum amount of unused top-most memory to keep before
releasing via malloc_trim in free().  Automatic trimming is mainly
useful in long-lived programs using contiguous MORECORE.  Because
trimming via sbrk can be slow on some systems, and can sometimes be
wasteful (in cases where programs immediately afterward allocate
more large chunks) the value should be high enough so that your
overall system performance would improve by releasing this much
memory.  As a rough guide, you might set to a value close to the
average size of a process (program) running on your system.
Releasing this much memory would allow such a process to run in
memory.  Generally, it is worth tuning trim thresholds when a
program undergoes phases where several large chunks are allocated
and released in ways that can reuse each other's storage, perhaps
mixed with phases where there are no such chunks at all. The trim
value must be greater than page size to have any useful effect.  To
disable trimming completely, you can set to MAX_SIZE_T. Note that the trick
some people use of mallocing a huge space and then freeing it at
program startup, in an attempt to reserve system memory, doesn't
have the intended effect under automatic trimming, since that memory
will immediately be returned to the system.

DEFAULT_MMAP_THRESHOLD       default: 256K
Also settable using mallopt(M_MMAP_THRESHOLD, x)
The request size threshold for using MMAP to directly service a
request. Requests of at least this size that cannot be allocated
using already-existing space will be serviced via mmap.  (If enough
normal freed space already exists it is used instead.)  Using mmap
segregates relatively large chunks of memory so that they can be
individually obtained and released from the host system. A request
serviced through mmap is never reused by any other request (at least
not directly; the system may just so happen to remap successive
requests to the same locations).  Segregating space in this way has
the benefits that: Mmapped space can always be individually released
back to the system, which helps keep the system level memory demands
of a long-lived program low.  Also, mapped memory doesn't become
`locked' between other chunks, as can happen with normally allocated
chunks, which means that even trimming via malloc_trim would not
release them.  However, it has the disadvantage that the space
cannot be reclaimed, consolidated, and then used to service later
requests, as happens with normal chunks.  The advantages of mmap
nearly always outweigh disadvantages for "large" chunks, but the
value of "large" may vary across systems.  The default is an
empirically derived value that works well in most systems. You can
disable mmap by setting to MAX_SIZE_T.

MAX_RELEASE_CHECK_RATE   default: 4095 unless not HAVE_MMAP
The number of consolidated frees between checks to release
unused segments when freeing. When using non-contiguous segments,
especially with multiple mspaces, checking only for topmost space
doesn't always suffice to trigger trimming. To compensate for this,
free() will, with a period of MAX_RELEASE_CHECK_RATE (or the
current number of segments, if greater) try to release unused
segments to the OS when freeing chunks that result in
consolidation. The best value for this parameter is a compromise
between slowing down frees with relatively costly checks that
rarely trigger versus holding on to unused memory. To effectively
disable, set to MAX_SIZE_T. This may lead to a very slight speed
improvement at the expense of carrying around more memory.
*/

/* Version identifier to allow people to support multiple versions */
#ifndef DLMALLOC_VERSION
#define DLMALLOC_VERSION 20804
#endif /* DLMALLOC_VERSION */

#include "rdlmalloc-options.h"

#ifndef WIN32
#if defined(_XBOX) || defined(X360)
#else
#if defined(_WIN32)
#define DL_PLATFORM_WIN32 1
#endif  /* _WIN32 */
#ifdef _WIN32_WCE
#define LACKS_FCNTL_H
#define DL_PLATFORM_WIN32 1
#endif /* _WIN32_WCE */
#endif
#else
#define DL_PLATFORM_WIN32 1
#endif  /* DL_PLATFORM_WIN32 */

#if defined(_XBOX) || defined(X360)
#define HAVE_MMAP 1
#define HAVE_MORECORE 0
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION
#endif
#define MMAP_CLEARS 1
#endif

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(SN_TARGET_PSP2)
#define LACKS_SYS_PARAM_H
#include "sysutil\sysutil_sysparam.h"
#define LACKS_SYS_MMAN_H
#endif


#ifdef DL_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define HAVE_MMAP 1
#define HAVE_MORECORE 0
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION
#endif /* MALLOC_FAILURE_ACTION */
#ifdef _WIN32_WCE /* WINCE reportedly does not clear */
#define MMAP_CLEARS 0
#else
#define MMAP_CLEARS 1
#endif /* _WIN32_WCE */
#endif  /* DL_PLATFORM_WIN32 */

#if defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#ifndef HAVE_MORECORE
#define HAVE_MORECORE 0
#define HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)16U)
#endif
#endif  /* HAVE_MORECORE */
#endif  /* DARWIN */

#ifndef LACKS_SYS_TYPES_H
#include <sys/types.h>  /* For size_t */
#endif  /* LACKS_SYS_TYPES_H */

#if (defined(__GNUC__) && ((defined(__i386__) || defined(__x86_64__)))) || (defined(_MSC_VER) && _MSC_VER>=1310)
#define SPIN_LOCKS_AVAILABLE 1
#else
#define SPIN_LOCKS_AVAILABLE 0
#endif

/* The maximum possible size_t value has all bits set */
#define MAX_SIZE_T           (~(size_t)0)

#ifndef ONLY_MSPACES
#define ONLY_MSPACES 0     /* define to a value */
#else
#define ONLY_MSPACES 1
#endif  /* ONLY_MSPACES */
#ifndef MSPACES
#if ONLY_MSPACES
#define MSPACES 1
#else   /* ONLY_MSPACES */
#define MSPACES 0
#endif  /* ONLY_MSPACES */
#endif  /* MSPACES */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)8U)
#endif  /* MALLOC_ALIGNMENT */
#ifndef FOOTERS
#define FOOTERS 0
#endif  /* FOOTERS */
#ifndef ABORT
#define ABORT  abort()
#endif  /* ABORT */
#ifndef ABORT_ON_ASSERT_FAILURE
#define ABORT_ON_ASSERT_FAILURE 1
#endif  /* ABORT_ON_ASSERT_FAILURE */
#ifndef PROCEED_ON_ERROR
#define PROCEED_ON_ERROR 0
#endif  /* PROCEED_ON_ERROR */
#ifndef USE_LOCKS
#define USE_LOCKS 0
#endif  /* USE_LOCKS */
#ifndef USE_SPIN_LOCKS
#if USE_LOCKS && SPIN_LOCKS_AVAILABLE
#define USE_SPIN_LOCKS 1
#else
#define USE_SPIN_LOCKS 0
#endif /* USE_LOCKS && SPIN_LOCKS_AVAILABLE. */
#endif /* USE_SPIN_LOCKS */
#ifndef INSECURE
#define INSECURE 0
#endif  /* INSECURE */
#ifndef HAVE_MMAP
#define HAVE_MMAP 1
#endif  /* HAVE_MMAP */
#ifndef MMAP_CLEARS
#define MMAP_CLEARS 1
#endif  /* MMAP_CLEARS */
#ifndef HAVE_MREMAP
#ifdef linux
#define HAVE_MREMAP 1
#else   /* linux */
#define HAVE_MREMAP 0
#endif  /* linux */
#endif  /* HAVE_MREMAP */
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION  errno = ENOMEM;
#endif  /* MALLOC_FAILURE_ACTION */
#ifndef HAVE_MORECORE
#if ONLY_MSPACES
#define HAVE_MORECORE 0
#else   /* ONLY_MSPACES */
#define HAVE_MORECORE 1
#endif  /* ONLY_MSPACES */
#endif  /* HAVE_MORECORE */
#if !HAVE_MORECORE
#define MORECORE_CONTIGUOUS 0
#else   /* !HAVE_MORECORE */
#define MORECORE_DEFAULT sbrk
#ifndef MORECORE_CONTIGUOUS
#define MORECORE_CONTIGUOUS 1
#endif  /* MORECORE_CONTIGUOUS */
#endif  /* HAVE_MORECORE */
#ifndef DEFAULT_GRANULARITY
#if (MORECORE_CONTIGUOUS || defined(DL_PLATFORM_WIN32))
#define DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#else   /* MORECORE_CONTIGUOUS */
#define DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#endif  /* MORECORE_CONTIGUOUS */
#endif  /* DEFAULT_GRANULARITY */
#ifndef DEFAULT_TRIM_THRESHOLD
#ifndef MORECORE_CANNOT_TRIM
#define DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#else   /* MORECORE_CANNOT_TRIM */
#define DEFAULT_TRIM_THRESHOLD MAX_SIZE_T
#endif  /* MORECORE_CANNOT_TRIM */
#endif  /* DEFAULT_TRIM_THRESHOLD */
#ifndef DEFAULT_MMAP_THRESHOLD
#if HAVE_MMAP
#define DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#else   /* HAVE_MMAP */
#define DEFAULT_MMAP_THRESHOLD MAX_SIZE_T
#endif  /* HAVE_MMAP */
#endif  /* DEFAULT_MMAP_THRESHOLD */
#ifndef MAX_RELEASE_CHECK_RATE
#if HAVE_MMAP
#define MAX_RELEASE_CHECK_RATE 4095
#else
#define MAX_RELEASE_CHECK_RATE MAX_SIZE_T
#endif /* HAVE_MMAP */
#endif /* MAX_RELEASE_CHECK_RATE */
#ifndef USE_BUILTIN_FFS
#define USE_BUILTIN_FFS 0
#endif  /* USE_BUILTIN_FFS */
#ifndef USE_DEV_RANDOM
#define USE_DEV_RANDOM 0
#endif  /* USE_DEV_RANDOM */
#ifndef NO_MALLINFO
#define NO_MALLINFO 0
#endif  /* NO_MALLINFO */
#ifndef MALLINFO_FIELD_TYPE
#define MALLINFO_FIELD_TYPE size_t
#endif  /* MALLINFO_FIELD_TYPE */
#ifndef NO_SEGMENT_TRAVERSAL
#define NO_SEGMENT_TRAVERSAL 0
#endif /* NO_SEGMENT_TRAVERSAL */

/*
mallopt tuning options.  SVID/XPG defines four standard parameter
numbers for mallopt, normally defined in malloc.h.  None of these
are used in this malloc, so setting them has no effect. But this
malloc does support the following options.
*/

#define M_TRIM_THRESHOLD     (-1)
#define M_GRANULARITY        (-2)
#define M_MMAP_THRESHOLD     (-3)

/* ------------------------ Mallinfo declarations ------------------------ */

#if !NO_MALLINFO
/*
This version of malloc supports the standard SVID/XPG mallinfo
routine that returns a struct containing usage properties and
statistics. It should work on any system that has a
/usr/include/malloc.h defining struct mallinfo.  The main
declaration needed is the mallinfo struct that is returned (by-copy)
by mallinfo().  The malloinfo struct contains a bunch of fields that
are not even meaningful in this version of malloc.  These fields are
are instead filled by mallinfo() with other numbers that might be of
interest.

HAVE_USR_INCLUDE_MALLOC_H should be set if you have a
/usr/include/malloc.h file that includes a declaration of struct
mallinfo.  If so, it is included; else a compliant version is
declared below.  These must be precisely the same for mallinfo() to
work.  The original SVID version of this struct, defined on most
systems with mallinfo, declares all fields as ints. But some others
define as unsigned long. If your system defines the fields using a
type of different width than listed here, you MUST #include your
system version and #define HAVE_USR_INCLUDE_MALLOC_H.
*/

/* #define HAVE_USR_INCLUDE_MALLOC_H */

#ifdef HAVE_USR_INCLUDE_MALLOC_H
#include "/usr/include/malloc.h"
#else /* HAVE_USR_INCLUDE_MALLOC_H */
#ifndef STRUCT_MALLINFO_DECLARED
#define STRUCT_MALLINFO_DECLARED 1
struct mallinfo {
	MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system */
	MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
	MALLINFO_FIELD_TYPE smblks;   /* always 0 */
	MALLINFO_FIELD_TYPE hblks;    /* always 0 */
	MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
	MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
	MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
	MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
	MALLINFO_FIELD_TYPE fordblks; /* total free space */
	MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#endif /* STRUCT_MALLINFO_DECLARED */
#endif /* HAVE_USR_INCLUDE_MALLOC_H */
#endif /* NO_MALLINFO */

/*
Try to persuade compilers to inline. The most critical functions for
inlining are defined as macros, so these aren't used for them.
*/

#ifndef FORCEINLINE
#if defined(__GNUC__)
#define FORCEINLINE __inline __attribute__ ((always_inline))
#elif defined(_MSC_VER)
#define FORCEINLINE __forceinline
#endif
#endif
#ifndef NOINLINE
#if defined(__GNUC__)
#define NOINLINE __attribute__ ((noinline))
#elif defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif
#endif

#ifdef __cplusplus
extern "C" {
#ifndef FORCEINLINE
#define FORCEINLINE inline
#endif
#endif /* __cplusplus */
#ifndef FORCEINLINE
#define FORCEINLINE
#endif

#if !ONLY_MSPACES

	/* ------------------- Declarations of public routines ------------------- */

#ifndef USE_DL_PREFIX
#define rdlcalloc               calloc
#define rdlfree                 free
#define rdlmalloc               malloc
#define rdlmemalign             memalign
#define rdlrealloc              realloc
#define rdlvalloc               valloc
#define rdlpvalloc              pvalloc
#define rdlmallinfo             mallinfo
#define rdlmallopt              mallopt
#define rdlmalloc_trim          malloc_trim
#define rdlmalloc_stats         malloc_stats
#define rdlmalloc_usable_size   malloc_usable_size
#define rdlmalloc_footprint     malloc_footprint
#define dlmalloc_max_footprint malloc_max_footprint
#define rdlindependent_calloc   independent_calloc
#define rdlindependent_comalloc independent_comalloc
#endif /* USE_DL_PREFIX */


	/*
	malloc(size_t n)
	Returns a pointer to a newly allocated chunk of at least n bytes, or
	null if no space is available, in which case errno is set to ENOMEM
	on ANSI C systems.

	If n is zero, malloc returns a minimum-sized chunk. (The minimum
	size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
	systems.)  Note that size_t is an unsigned type, so calls with
	arguments that would be negative if signed are interpreted as
	requests for huge amounts of space, which will often fail. The
	maximum supported value of n differs across systems, but is in all
	cases less than the maximum representable value of a size_t.
	*/
	void* rdlmalloc(size_t);

	/*
	free(void* p)
	Releases the chunk of memory pointed to by p, that had been previously
	allocated using malloc or a related routine such as realloc.
	It has no effect if p is null. If p was not malloced or already
	freed, free(p) will by default cause the current program to abort.
	*/
	void  rdlfree(void*);

	/*
	calloc(size_t n_elements, size_t element_size);
	Returns a pointer to n_elements * element_size bytes, with all locations
	set to zero.
	*/
	void* rdlcalloc(size_t, size_t);

	/*
	realloc(void* p, size_t n)
	Returns a pointer to a chunk of size n that contains the same data
	as does chunk p up to the minimum of (n, p's size) bytes, or null
	if no space is available.

	The returned pointer may or may not be the same as p. The algorithm
	prefers extending p in most cases when possible, otherwise it
	employs the equivalent of a malloc-copy-free sequence.

	If p is null, realloc is equivalent to malloc.

	If space is not available, realloc returns null, errno is set (if on
	ANSI) and p is NOT freed.

	if n is for fewer bytes than already held by p, the newly unused
	space is lopped off and freed if possible.  realloc with a size
	argument of zero (re)allocates a minimum-sized chunk.

	The old unix realloc convention of allowing the last-free'd chunk
	to be used as an argument to realloc is not supported.
	*/

	void* rdlrealloc(void*, size_t);

	/*
	memalign(size_t alignment, size_t n);
	Returns a pointer to a newly allocated chunk of n bytes, aligned
	in accord with the alignment argument.

	The alignment argument should be a power of two. If the argument is
	not a power of two, the nearest greater power is used.
	8-byte alignment is guaranteed by normal malloc calls, so don't
	bother calling memalign with an argument of 8 or less.

	Overreliance on memalign is a sure way to fragment space.
	*/
	void* rdlmemalign(size_t, size_t);

	/*
	valloc(size_t n);
	Equivalent to memalign(pagesize, n), where pagesize is the page
	size of the system. If the pagesize is unknown, 4096 is used.
	*/
	void* rdlvalloc(size_t);

	/*
	mallopt(int parameter_number, int parameter_value)
	Sets tunable parameters The format is to provide a
	(parameter-number, parameter-value) pair.  mallopt then sets the
	corresponding parameter to the argument value if it can (i.e., so
	long as the value is meaningful), and returns 1 if successful else
	0.  To workaround the fact that mallopt is specified to use int,
	not size_t parameters, the value -1 is specially treated as the
	maximum unsigned size_t value.

	SVID/XPG/ANSI defines four standard param numbers for mallopt,
	normally defined in malloc.h.  None of these are use in this malloc,
	so setting them has no effect. But this malloc also supports other
	options in mallopt. See below for details.  Briefly, supported
	parameters are as follows (listed defaults are for "typical"
	configurations).

	Symbol            param #  default    allowed param values
	M_TRIM_THRESHOLD     -1   2*1024*1024   any   (-1 disables)
	M_GRANULARITY        -2     page size   any power of 2 >= page size
	M_MMAP_THRESHOLD     -3      256*1024   any   (or 0 if no MMAP support)
	*/
	int rdlmallopt(int, int);

	/*
	malloc_footprint();
	Returns the number of bytes obtained from the system.  The total
	number of bytes allocated by malloc, realloc etc., is less than this
	value. Unlike mallinfo, this function returns only a precomputed
	result, so can be called frequently to monitor memory consumption.
	Even if locks are otherwise defined, this function does not use them,
	so results might not be up to date.
	*/
	size_t rdlmalloc_footprint(void);

	/*
	malloc_max_footprint();
	Returns the maximum number of bytes obtained from the system. This
	value will be greater than current footprint if deallocated space
	has been reclaimed by the system. The peak number of bytes allocated
	by malloc, realloc etc., is less than this value. Unlike mallinfo,
	this function returns only a precomputed result, so can be called
	frequently to monitor memory consumption.  Even if locks are
	otherwise defined, this function does not use them, so results might
	not be up to date.
	*/
	size_t dlmalloc_max_footprint(void);

#if !NO_MALLINFO
	/*
	mallinfo()
	Returns (by copy) a struct containing various summary statistics:

	arena:     current total non-mmapped bytes allocated from system
	ordblks:   the number of free chunks
	smblks:    always zero.
	hblks:     current number of mmapped regions
	hblkhd:    total bytes held in mmapped regions
	usmblks:   the maximum total allocated space. This will be greater
	than current total if trimming has occurred.
	fsmblks:   always zero
	uordblks:  current total allocated space (normal or mmapped)
	fordblks:  total free space
	keepcost:  the maximum number of bytes that could ideally be released
	back to system via malloc_trim. ("ideally" means that
	it ignores page restrictions etc.)

	Because these fields are ints, but internal bookkeeping may
	be kept as longs, the reported values may wrap around zero and
	thus be inaccurate.
	*/
	struct mallinfo rdlmallinfo(void);
#endif /* NO_MALLINFO */

	/*
	independent_calloc(size_t n_elements, size_t element_size, void* chunks[]);

	independent_calloc is similar to calloc, but instead of returning a
	single cleared space, it returns an array of pointers to n_elements
	independent elements that can hold contents of size elem_size, each
	of which starts out cleared, and can be independently freed,
	realloc'ed etc. The elements are guaranteed to be adjacently
	allocated (this is not guaranteed to occur with multiple callocs or
	mallocs), which may also improve cache locality in some
	applications.

	The "chunks" argument is optional (i.e., may be null, which is
	probably the most typical usage). If it is null, the returned array
	is itself dynamically allocated and should also be freed when it is
	no longer needed. Otherwise, the chunks array must be of at least
	n_elements in length. It is filled in with the pointers to the
	chunks.

	In either case, independent_calloc returns this pointer array, or
	null if the allocation failed.  If n_elements is zero and "chunks"
	is null, it returns a chunk representing an array with zero elements
	(which should be freed if not wanted).

	Each element must be individually freed when it is no longer
	needed. If you'd like to instead be able to free all at once, you
	should instead use regular calloc and assign pointers into this
	space to represent elements.  (In this case though, you cannot
	independently free elements.)

	independent_calloc simplifies and speeds up implementations of many
	kinds of pools.  It may also be useful when constructing large data
	structures that initially have a fixed number of fixed-sized nodes,
	but the number is not known at compile time, and some of the nodes
	may later need to be freed. For example:

	struct Node { int item; struct Node* next; };

	struct Node* build_list() {
	struct Node** pool;
	int n = read_number_of_nodes_needed();
	if (n <= 0) return 0;
	pool = (struct Node**)(independent_calloc(n, sizeof(struct Node), 0);
	if (pool == 0) die();
	// organize into a linked list...
	struct Node* first = pool[0];
	for (i = 0; i < n-1; ++i)
	pool[i]->next = pool[i+1];
	free(pool);     // Can now free the array (or not, if it is needed later)
	return first;
	}
	*/
	void** rdlindependent_calloc(size_t, size_t, void**);

	/*
	independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);

	independent_comalloc allocates, all at once, a set of n_elements
	chunks with sizes indicated in the "sizes" array.    It returns
	an array of pointers to these elements, each of which can be
	independently freed, realloc'ed etc. The elements are guaranteed to
	be adjacently allocated (this is not guaranteed to occur with
	multiple callocs or mallocs), which may also improve cache locality
	in some applications.

	The "chunks" argument is optional (i.e., may be null). If it is null
	the returned array is itself dynamically allocated and should also
	be freed when it is no longer needed. Otherwise, the chunks array
	must be of at least n_elements in length. It is filled in with the
	pointers to the chunks.

	In either case, independent_comalloc returns this pointer array, or
	null if the allocation failed.  If n_elements is zero and chunks is
	null, it returns a chunk representing an array with zero elements
	(which should be freed if not wanted).

	Each element must be individually freed when it is no longer
	needed. If you'd like to instead be able to free all at once, you
	should instead use a single regular malloc, and assign pointers at
	particular offsets in the aggregate space. (In this case though, you
	cannot independently free elements.)

	independent_comallac differs from independent_calloc in that each
	element may have a different size, and also that it does not
	automatically clear elements.

	independent_comalloc can be used to speed up allocation in cases
	where several structs or objects must always be allocated at the
	same time.  For example:

	struct Head { ... }
	struct Foot { ... }

	void send_message(char* msg) {
	int msglen = strlen(msg);
	size_t sizes[3] = { sizeof(struct Head), msglen, sizeof(struct Foot) };
	void* chunks[3];
	if (independent_comalloc(3, sizes, chunks) == 0)
	die();
	struct Head* head = (struct Head*)(chunks[0]);
	char*        body = (char*)(chunks[1]);
	struct Foot* foot = (struct Foot*)(chunks[2]);
	// ...
	}

	In general though, independent_comalloc is worth using only for
	larger values of n_elements. For small values, you probably won't
	detect enough difference from series of malloc calls to bother.

	Overuse of independent_comalloc can increase overall memory usage,
	since it cannot reuse existing noncontiguous small chunks that
	might be available for some of the elements.
	*/
	void** rdlindependent_comalloc(size_t, size_t*, void**);


	/*
	pvalloc(size_t n);
	Equivalent to valloc(minimum-page-that-holds(n)), that is,
	round up n to nearest pagesize.
	*/
	void*  rdlpvalloc(size_t);

	/*
	malloc_trim(size_t pad);

	If possible, gives memory back to the system (via negative arguments
	to sbrk) if there is unused memory at the `high' end of the malloc
	pool or in unused MMAP segments. You can call this after freeing
	large blocks of memory to potentially reduce the system-level memory
	requirements of a program. However, it cannot guarantee to reduce
	memory. Under some allocation patterns, some large free blocks of
	memory will be locked between two used chunks, so they cannot be
	given back to the system.

	The `pad' argument to malloc_trim represents the amount of free
	trailing space to leave untrimmed. If this argument is zero, only
	the minimum amount of memory to maintain internal data structures
	will be left. Non-zero arguments can be supplied to maintain enough
	trailing space to service future expected allocations without having
	to re-obtain memory from the system.

	Malloc_trim returns 1 if it actually released any memory, else 0.
	*/
	int  rdlmalloc_trim(size_t);

	/*
	malloc_stats();
	Prints on stderr the amount of space obtained from the system (both
	via sbrk and mmap), the maximum amount (which may be more than
	current if malloc_trim and/or munmap got called), and the current
	number of bytes allocated via malloc (or realloc, etc) but not yet
	freed. Note that this is the number of bytes allocated, not the
	number requested. It will be larger than the number requested
	because of alignment and bookkeeping overhead. Because it includes
	alignment wastage as being in use, this figure may be greater than
	zero even when no user-level chunks are allocated.

	The reported current and maximum system memory can be inaccurate if
	a program makes other calls to system memory allocation functions
	(normally sbrk) outside of malloc.

	malloc_stats prints only the most commonly interesting statistics.
	More information can be obtained by calling mallinfo.
	*/
	void  rdlmalloc_stats(void);

#endif /* ONLY_MSPACES */

	/*
	malloc_usable_size(void* p);

	Returns the number of bytes you can actually use in
	an allocated chunk, which may be more than you requested (although
	often not) due to alignment and minimum size constraints.
	You can use this many bytes without worrying about
	overwriting other allocated objects. This is not a particularly great
	programming practice. malloc_usable_size can be more useful in
	debugging and assertions, for example:

	p = malloc(n);
	assert(malloc_usable_size(p) >= 256);
	*/
	size_t rdlmalloc_usable_size(void*);


#if MSPACES

	/*
	mspace is an opaque type representing an independent
	region of space that supports rak_mspace_malloc, etc.
	*/
	typedef void* mspace;

	/*
	rak_create_mspace creates and returns a new independent space with the
	given initial capacity, or, if 0, the default granularity size.  It
	returns null if there is no system memory available to create the
	space.  If argument locked is non-zero, the space uses a separate
	lock to control access. The capacity of the space will grow
	dynamically as needed to service rak_mspace_malloc requests.  You can
	control the sizes of incremental increases of this space by
	compiling with a different DEFAULT_GRANULARITY or dynamically
	setting with mallopt(M_GRANULARITY, value).
	*/
	mspace rak_create_mspace(size_t capacity, int locked);

	/*
	rak_destroy_mspace destroys the given space, and attempts to return all
	of its memory back to the system, returning the total number of
	bytes freed. After destruction, the results of access to all memory
	used by the space become undefined.
	*/
	size_t rak_destroy_mspace(mspace msp);

	/*
	rak_create_mspace_with_base uses the memory supplied as the initial base
	of a new mspace. Part (less than 128*sizeof(size_t) bytes) of this
	space is used for bookkeeping, so the capacity must be at least this
	large. (Otherwise 0 is returned.) When this initial space is
	exhausted, additional memory will be obtained from the system.
	Destroying this space will deallocate all additionally allocated
	space (if possible) but not the initial base.
	*/
	mspace rak_create_mspace_with_base(void* base, size_t capacity, int locked);

	/*
	rak_mspace_track_large_chunks controls whether requests for large chunks
	are allocated in their own untracked mmapped regions, separate from
	others in this mspace. By default large chunks are not tracked,
	which reduces fragmentation. However, such chunks are not
	necessarily released to the system upon rak_destroy_mspace.  Enabling
	tracking by setting to true may increase fragmentation, but avoids
	leakage when relying on rak_destroy_mspace to release all memory
	allocated using this space.  The function returns the previous
	setting.
	*/
	int rak_mspace_track_large_chunks(mspace msp, int enable);


	/*
	rak_mspace_malloc behaves as malloc, but operates within
	the given space.
	*/
	void* rak_mspace_malloc(mspace msp, size_t bytes);

	/*
	rak_mspace_free behaves as free, but operates within
	the given space.

	If compiled with FOOTERS==1, rak_mspace_free is not actually needed.
	free may be called instead of rak_mspace_free because freed chunks from
	any space are handled by their originating spaces.
	*/
	void rak_mspace_free(mspace msp, void* mem);

	/*
	rak_mspace_realloc behaves as realloc, but operates within
	the given space.

	If compiled with FOOTERS==1, rak_mspace_realloc is not actually
	needed.  realloc may be called instead of rak_mspace_realloc because
	realloced chunks from any space are handled by their originating
	spaces.
	*/
	void* rak_mspace_realloc(mspace msp, void* mem, size_t newsize);

	/*
	rak_mspace_calloc behaves as calloc, but operates within
	the given space.
	*/
	void* rak_mspace_calloc(mspace msp, size_t n_elements, size_t elem_size);

	/*
	rak_mspace_memalign behaves as memalign, but operates within
	the given space.
	*/
	void* rak_mspace_memalign(mspace msp, size_t alignment, size_t bytes);

	/*
	rak_mspace_independent_calloc behaves as independent_calloc, but
	operates within the given space.
	*/
	void** rak_mspace_independent_calloc(mspace msp, size_t n_elements,
		size_t elem_size, void* chunks[]);

	/*
	rak_mspace_independent_comalloc behaves as independent_comalloc, but
	operates within the given space.
	*/
	void** rak_mspace_independent_comalloc(mspace msp, size_t n_elements,
		size_t sizes[], void* chunks[]);

	/*
	rak_mspace_footprint() returns the number of bytes obtained from the
	system for this space.
	*/
	size_t rak_mspace_footprint(mspace msp);

	/*
	mspace_max_footprint() returns the peak number of bytes obtained from the
	system for this space.
	*/
	size_t mspace_max_footprint(mspace msp);


#if !NO_MALLINFO
	/*
	rak_mspace_mallinfo behaves as mallinfo, but reports properties of
	the given space.
	*/
	struct mallinfo rak_mspace_mallinfo(mspace msp);
#endif /* NO_MALLINFO */

	/*
	malloc_usable_size(void* p) behaves the same as malloc_usable_size;
	*/
	size_t rak_mspace_usable_size(void* mem);

	/*
	rak_mspace_malloc_stats behaves as malloc_stats, but reports
	properties of the given space.
	*/
	void rak_mspace_malloc_stats(mspace msp);

	/*
	rak_mspace_trim behaves as malloc_trim, but
	operates within the given space.
	*/
	int rak_mspace_trim(mspace msp, size_t pad);

	/*
	An alias for mallopt.
	*/
	int rak_mspace_mallopt(int, int);

#endif /* MSPACES */

#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */

/*
========================================================================
To make a fully customizable malloc.h header file, cut everything
above this line, put into file malloc.h, edit to suit, and #include it
on the next line, as well as in programs that use this malloc.
========================================================================
*/

/* #include "malloc.h" */

/*------------------------------ internal #includes ---------------------- */

#ifdef DL_PLATFORM_WIN32
#pragma warning( disable : 4146 ) /* no "unsigned" warnings */
#endif /* DL_PLATFORM_WIN32 */

#include <stdio.h>       /* for printing in malloc_stats */

#ifndef LACKS_ERRNO_H
#include <errno.h>       /* for MALLOC_FAILURE_ACTION */
#endif /* LACKS_ERRNO_H */

#if FOOTERS || DEBUG
#include <time.h>        /* for magic initialization */
#endif /* FOOTERS */

#ifndef LACKS_STDLIB_H
#include <stdlib.h>      /* for abort() */
#endif /* LACKS_STDLIB_H */

#ifdef DEBUG
#if ABORT_ON_ASSERT_FAILURE
#undef assert
#define assert(x) if(!(x)) ABORT
#else /* ABORT_ON_ASSERT_FAILURE */
#include <assert.h>
#endif /* ABORT_ON_ASSERT_FAILURE */
#else  /* DEBUG */
#ifndef assert
#define assert(x)
#endif
#define DEBUG 0
#endif /* DEBUG */

#ifndef LACKS_STRING_H
#include <string.h>      /* for memset etc */
#endif  /* LACKS_STRING_H */

#if USE_BUILTIN_FFS
#ifndef LACKS_STRINGS_H
#include <strings.h>     /* for ffs */
#endif /* LACKS_STRINGS_H */
#endif /* USE_BUILTIN_FFS */

#if HAVE_MMAP
#ifndef LACKS_SYS_MMAN_H
/* On some versions of linux, mremap decl in mman.h needs __USE_GNU set */
#if (defined(linux) && !defined(__USE_GNU))
#define __USE_GNU 1
#include <sys/mman.h>    /* for mmap */
#undef __USE_GNU
#else
#include <sys/mman.h>    /* for mmap */
#endif /* linux */
#endif /* LACKS_SYS_MMAN_H */
#ifndef LACKS_FCNTL_H
#include <fcntl.h>
#endif /* LACKS_FCNTL_H */
#endif /* HAVE_MMAP */

#ifndef LACKS_UNISTD_H
#include <unistd.h>     /* for sbrk, sysconf */
#else /* LACKS_UNISTD_H */
#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
extern void*     sbrk(ptrdiff_t);
#endif /* FreeBSD etc */
#endif /* LACKS_UNISTD_H */

/* Declarations for locking */
#if USE_LOCKS
#if defined(_XBOX) || defined(X360)
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedExchange)
#define interlockedcompareexchange _InterlockedCompareExchange
#define interlockedexchange _InterlockedExchange
#elif !defined(DL_PLATFORM_WIN32)
#include <pthread.h>
#if defined (__SVR4) && defined (__sun)  /* solaris */
#include <thread.h>
#endif /* solaris */
#else
#ifndef _M_AMD64
/* These are already defined on AMD64 builds */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	LONG __cdecl _InterlockedCompareExchange(LONG volatile *Dest, LONG Exchange, LONG Comp);
	LONG __cdecl _InterlockedExchange(LONG volatile *Target, LONG Value);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _M_AMD64 */
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedExchange)
#define interlockedcompareexchange _InterlockedCompareExchange
#define interlockedexchange _InterlockedExchange
#endif /* Win32 */
#endif /* USE_LOCKS */

/* Declarations for bit scanning on win32 */
#if defined(_MSC_VER) && _MSC_VER>=1300 && defined(DL_PLATFORM_WIN32)
#ifndef BitScanForward	/* Try to avoid pulling in WinNT.h */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	unsigned char _BitScanForward(unsigned long *index, unsigned long mask);
	unsigned char _BitScanReverse(unsigned long *index, unsigned long mask);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#define BitScanForward _BitScanForward
#define BitScanReverse _BitScanReverse
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#endif /* BitScanForward */
#endif /* defined(_MSC_VER) && _MSC_VER>=1300 */

#ifndef DL_PLATFORM_WIN32
#ifndef malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef _SC_PAGE_SIZE
#      define _SC_PAGE_SIZE _SC_PAGESIZE
#    endif
#  endif
#  ifdef _SC_PAGE_SIZE
#    define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#  else
#    if defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
extern size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    else
#      ifdef DL_PLATFORM_WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize()
#      else
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        else
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            else
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          else
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            else
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              else /* just guess */
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#endif
#endif



/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#define SIZE_T_SIZE         (sizeof(size_t))
#define SIZE_T_BITSIZE      (sizeof(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#define SIZE_T_ZERO         ((size_t)0)
#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZE_T_FOUR         ((size_t)4)
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
#define FOUR_SIZE_T_SIZES   (SIZE_T_SIZE<<2)
#define SIX_SIZE_T_SIZES    (FOUR_SIZE_T_SIZES+TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T     (MAX_SIZE_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)

/* True if address a has acceptable alignment */
#define is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

/* the number of bytes to offset an address to align it */
#define align_offset(A)\
	((((size_t)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :\
	((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))

/* -------------------------- MMAP preliminaries ------------------------- */

/*
If HAVE_MORECORE or HAVE_MMAP are false, we just define calls and
checks to fail so compiler optimizer can delete code rather than
using so many "#if"s.
*/


/* MORECORE and MMAP must return MFAIL on failure */
#define MFAIL                ((void*)(MAX_SIZE_T))
#define CMFAIL               ((char*)(MFAIL)) /* defined for convenience */

#if HAVE_MMAP

#if defined(_XBOX) || defined(X360)
	/* Win32 MMAP via VirtualAlloc */
	static void* win32mmap(size_t size) {
		void* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		return (ptr != 0)? ptr: MFAIL;
	}

	/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
	static void* win32direct_mmap(size_t size) {
		void* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,
			PAGE_READWRITE);
		return (ptr != 0)? ptr: MFAIL;
	}

	/* This function supports releasing coalesed segments */
	static int win32munmap(void* ptr, size_t size) {
		MEMORY_BASIC_INFORMATION minfo;
		char* cptr = (char*)ptr;
		while (size) {
			if (VirtualQuery(cptr, &minfo, sizeof(minfo)) == 0)
				return -1;
			if (minfo.BaseAddress != cptr || minfo.AllocationBase != cptr ||
				minfo.State != MEM_COMMIT || minfo.RegionSize > size)
				return -1;
			if (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
				return -1;
			cptr += minfo.RegionSize;
			size -= minfo.RegionSize;
		}
		return 0;
	}

	#define RAK_MMAP_DEFAULT(s)             win32mmap(s)
	#define RAK_MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
	#define RAK_DIRECT_MMAP_DEFAULT(s)      win32direct_mmap(s)
#elif defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(SN_TARGET_PSP2)

inline int ___freeit_dlmalloc_default__(void* s) {free(s); return 0;}
#define RAK_MMAP_DEFAULT(s) malloc(s);
#define RAK_MUNMAP_DEFAULT(a, s) ___freeit_dlmalloc_default__(a);
#define RAK_DIRECT_MMAP_DEFAULT(s) malloc(s);

#elif !defined(DL_PLATFORM_WIN32)
	#define RAK_MUNMAP_DEFAULT(a, s)  munmap((a), (s))
	#define MMAP_PROT            (PROT_READ|PROT_WRITE)
	#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
	#define MAP_ANONYMOUS        MAP_ANON
	#endif /* MAP_ANON */
	#ifdef MAP_ANONYMOUS
	#define MMAP_FLAGS           (MAP_PRIVATE|MAP_ANONYMOUS)
	#define RAK_MMAP_DEFAULT(s)       mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
	#else /* MAP_ANONYMOUS */
	/*
	Nearly all versions of mmap support MAP_ANONYMOUS, so the following
	is unlikely to be needed, but is supplied just in case.
	*/
	#define MMAP_FLAGS           (MAP_PRIVATE)
	static int dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
	#define RAK_MMAP_DEFAULT(s) ((dev_zero_fd < 0) ? \
		(dev_zero_fd = open("/dev/zero", O_RDWR), \
		mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
		mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
	#endif /* MAP_ANONYMOUS */

	#define RAK_DIRECT_MMAP_DEFAULT(s) RAK_MMAP_DEFAULT(s)

#else /* DL_PLATFORM_WIN32 */

	/* Win32 MMAP via VirtualAlloc */
	static FORCEINLINE void* win32mmap(size_t size) {
		void* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		return (ptr != 0)? ptr: MFAIL;
	}

	/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
	static FORCEINLINE void* win32direct_mmap(size_t size) {
		void* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,
			PAGE_READWRITE);
		return (ptr != 0)? ptr: MFAIL;
	}

	/* This function supports releasing coalesed segments */
	static FORCEINLINE int win32munmap(void* ptr, size_t size) {
		MEMORY_BASIC_INFORMATION minfo;
		char* cptr = (char*)ptr;
		while (size) {
			if (VirtualQuery(cptr, &minfo, sizeof(minfo)) == 0)
				return -1;
			if (minfo.BaseAddress != cptr || minfo.AllocationBase != cptr ||
				minfo.State != MEM_COMMIT || minfo.RegionSize > size)
				return -1;
			if (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
				return -1;
			cptr += minfo.RegionSize;
			size -= minfo.RegionSize;
		}
		return 0;
	}

	#define RAK_MMAP_DEFAULT(s)             win32mmap(s)
	#define RAK_MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
	#define RAK_DIRECT_MMAP_DEFAULT(s)      win32direct_mmap(s)
#endif /* DL_PLATFORM_WIN32 */
#endif /* HAVE_MMAP */

#if HAVE_MREMAP
#ifndef DL_PLATFORM_WIN32
#define MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#endif /* DL_PLATFORM_WIN32 */
#endif /* HAVE_MREMAP */


/**
* Define CALL_MORECORE
*/
#if HAVE_MORECORE
#ifdef MORECORE
#define CALL_MORECORE(S)    MORECORE(S)
#else  /* MORECORE */
#define CALL_MORECORE(S)    MORECORE_DEFAULT(S)
#endif /* MORECORE */
#else  /* HAVE_MORECORE */
#define CALL_MORECORE(S)        MFAIL
#endif /* HAVE_MORECORE */

/**
* Define CALL_MMAP/CALL_MUNMAP/CALL_DIRECT_MMAP
*/
#if HAVE_MMAP
#define USE_MMAP_BIT            (SIZE_T_ONE)

#ifdef MMAP
#define CALL_MMAP(s)        MMAP(s)
#else /* MMAP */
#define CALL_MMAP(s)        RAK_MMAP_DEFAULT(s)
#endif /* MMAP */
#ifdef MUNMAP
#define CALL_MUNMAP(a, s)   MUNMAP((a), (s))
#else /* MUNMAP */
#define CALL_MUNMAP(a, s)   RAK_MUNMAP_DEFAULT((a), (s))
#endif /* MUNMAP */
#ifdef DIRECT_MMAP
#define CALL_DIRECT_MMAP(s) DIRECT_MMAP(s)
#else /* DIRECT_MMAP */
#define CALL_DIRECT_MMAP(s) RAK_DIRECT_MMAP_DEFAULT(s)
#endif /* DIRECT_MMAP */
#else  /* HAVE_MMAP */
#define USE_MMAP_BIT            (SIZE_T_ZERO)

#define MMAP(s)                 MFAIL
#define MUNMAP(a, s)            (-1)
#define DIRECT_MMAP(s)          MFAIL
#define CALL_DIRECT_MMAP(s)     DIRECT_MMAP(s)
#define CALL_MMAP(s)            MMAP(s)
#define CALL_MUNMAP(a, s)       MUNMAP((a), (s))
#endif /* HAVE_MMAP */

/**
* Define CALL_MREMAP
*/
#if HAVE_MMAP && HAVE_MREMAP
#ifdef MREMAP
#define CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
#else /* MREMAP */
#define CALL_MREMAP(addr, osz, nsz, mv) MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
#endif /* MREMAP */
#else  /* HAVE_MMAP && HAVE_MREMAP */
#define CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#endif /* HAVE_MMAP && HAVE_MREMAP */

/* mstate bit set if continguous morecore disabled or failed */
#define USE_NONCONTIGUOUS_BIT (4U)

/* segment bit set in rak_create_mspace_with_base */
#define EXTERN_BIT            (8U)


#endif /* MALLOC_280_H */

#endif // _RAKNET_SUPPORT_DL_MALLOC