#include "Debug.h"

Debug::Debug(const char* module)
{
	char buf[32];
	GetTimeFormat(buf, sizeof(buf), true);

	this->logfile = module;
	this->logfile += "_";
	this->logfile += buf;
	this->logfile += ".log";

	this->vaultmplog = fopen(logfile.c_str(), "w");
}

Debug::~Debug()
{
	if (this->vaultmplog != nullptr)
	{
		this->Print("-----------------------------------------------------------------------------------------------------", false);
		this->Print("END OF LOG", false);

		fclose(vaultmplog);
	}
}

void Debug::GetTimeFormat(char* buf, unsigned int size, bool file)
{
	time_t ltime;
	ltime = time(nullptr);

	tm* local = localtime(&ltime);
	char timeformat[32];
	snprintf(timeformat, sizeof(timeformat), "%d%c%02d%c%02d%c%02d%c%02d%c%02d", local->tm_year + 1900, '-', local->tm_mon + 1, '-', local->tm_mday, file ? '_' : ' ', local->tm_hour, file ? '-' : ':', local->tm_min, file ? '-' : ':', local->tm_sec);

	if (size > strlen(timeformat))
		strcpy(buf, timeformat);
}

void Debug::Print(const char* text, bool timestamp)
{
	StartSession();

	if (vaultmplog == nullptr)
		return;

	if (timestamp)
	{
		char buf[32];
		GetTimeFormat(buf, sizeof(buf), false);
		fprintf(this->vaultmplog, "[%s] ", buf);
	}

	/*
	char* lpMsgBuf;

	if( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) &lpMsgBuf, 0, NULL ) )
	{
		fprintf( this->vaultmplog, "[error: %s(0x%x)] ", lpMsgBuf, GetLastError() );
	}
	*/

	fwrite(text, sizeof(char), strlen(text), this->vaultmplog);
	fputc((int) '\n', this->vaultmplog);

	EndSession();
}

void Debug::PrintFormat(const char* format, bool timestamp, ...)
{
	char text[256];
	ZeroMemory(text, sizeof(text));

	va_list args;
	va_start(args, timestamp);
	vsnprintf(text, sizeof(text), format, args);
	va_end(args);

	Print(text, timestamp);
}

void Debug::PrintSystem()
{
	StartSession();

	FILE* systeminfo = popen("systeminfo", "r");

	if (systeminfo == nullptr)
		return;

	char buf[2048];

	while (fgets(buf, sizeof(buf), systeminfo) != nullptr)
		fwrite(buf, sizeof(char), strlen(buf), this->vaultmplog);

	pclose(systeminfo);

	EndSession();
}
