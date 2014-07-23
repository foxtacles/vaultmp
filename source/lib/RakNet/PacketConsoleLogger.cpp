#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_LogCommandParser==1 && _RAKNET_SUPPORT_PacketLogger==1
#include "PacketConsoleLogger.h"
#include "LogCommandParser.h"
#include <stdio.h>

using namespace RakNet;

PacketConsoleLogger::PacketConsoleLogger()
{
	logCommandParser=0;
}

void PacketConsoleLogger::SetLogCommandParser(LogCommandParser *lcp)
{
	logCommandParser=lcp;
	if (logCommandParser)
		logCommandParser->AddChannel("PacketConsoleLogger");
}
void PacketConsoleLogger::WriteLog(const char *str)
{
	if (logCommandParser)
		logCommandParser->WriteLog("PacketConsoleLogger", str);
}

#endif // _RAKNET_SUPPORT_*
