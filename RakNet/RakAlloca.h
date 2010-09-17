#if defined(__FreeBSD__)
#include <stdlib.h>
#elif defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3)
                   
#elif defined(_XBOX) || defined(X360)
      
#elif defined ( __APPLE__ ) || defined ( __APPLE_CC__ )
#include <malloc/malloc.h>
#include <alloca.h>
#elif defined(_WIN32)
#include <malloc.h>
#else
#include <malloc.h>
// Alloca needed on Ubuntu apparently
#include <alloca.h>
#endif
