#if   defined(GFWL)

#ifndef XBox360Includes_h__
#define XBox360Includes_h__

#include "WindowsIncludes.h"
#include <Xtl.h>
#include <stdio.h>
#include "RakSleep.h"

#if defined(GFWL) && !defined(XUSER_MAX_COUNT)
#define XUSER_MAX_COUNT 4
#endif

#if defined(GFWL)
#include "winlive.h"
#endif

inline void X360Startup(void)
{
	XNetStartupParams xnsp;
	memset(&xnsp,0,sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(xnsp);
#if XBOX_BYPASS_SECURITY==1
	#if defined(GFWL)
	xnsp.cfgFlags = XNET_STARTUP_DISABLE_PEER_ENCRYPTION;
	#else
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	#endif
#else
	xnsp.cfgFlags = 0;
#endif
	int result = XNetStartup(&xnsp);
	if (result != 0)
	{
		printf("XNetStartup failed: %i\n", result);
	}
}

inline void X360Shutdown(void) {}

inline void GetMACAddress(unsigned char buff[6])
{
	XNADDR xna;
	while (XNetGetTitleXnAddr(&xna)==XNET_GET_XNADDR_PENDING)
		RakSleep(1);
	for (int i=0; i < 6; i++)
		buff[i]=xna.abEnet[i];
}

#endif // XBox360Includes_h__

#endif
