#include "Debug.h"

using namespace std;

unique_ptr<Debug> Debug::debug;

Debug::Debug(const char* module)
{
	char buf[32];
	GetTimeFormat(buf, sizeof(buf), true);

	string logfile(module);
	logfile += "_" + string(buf) + ".log";

	this->vaultmplog.open(logfile.c_str(), fstream::out | fstream::trunc);
}

Debug::~Debug()
{
	this->Note("-----------------------------------------------------------------------------------------------------");
	this->Note("END OF LOG");
	this->vaultmplog.close();
}

void Debug::SetDebugHandler(const char* module)
{
	if (module)
		Debug::debug.reset(new Debug(module));
	else
		Debug::debug.reset();
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
