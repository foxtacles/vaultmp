#ifndef VS_MACRO_H_INCLUDED
#define VS_MACRO_H_INCLUDED

#ifndef _MSC_VER
#include <assert.h>
#define LPDIENUMEFFECTSCALLBACK LPDIENUMEFFECTSCALLBACKA
#define _ASSERTE(b) assert(b)
#define _ReturnAddress() __builtin_return_address(0)
#endif

#endif // VS_MACRO_H_INCLUDED
