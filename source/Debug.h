#ifndef DEBUG_H
#define DEBUG_H

#ifdef __WIN32__
#include <winsock2.h>
#endif
#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <cstring>

#include <string>

#include "vaultmp.h"
#include "CriticalSection.h"

using namespace std;

class Debug : private CriticalSection
{
	private:
		string logfile;
		FILE* vaultmplog;

		static void GetTimeFormat(char* buf, unsigned int size, bool file);

		Debug(const Debug&);
		Debug& operator=(const Debug&);

	public:
		Debug(const char* module);
		~Debug();

		void Print(const char* text, bool timestamp);
		void PrintFormat(const char* format, bool timestamp, ...);
		void PrintSystem();

};

#endif
