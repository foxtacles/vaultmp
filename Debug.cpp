#include "Debug.h"

Debug::Debug(char* module)
{
    char buf[32];
    GetTimeFormat(buf, sizeof(buf), true);

    this->logfile = module;
    this->logfile += "_";
    this->logfile += buf;
    this->logfile += ".log";

    this->vaultmplog = fopen(logfile.c_str(), "w");
    filemutex = false;
}

Debug::~Debug()
{
    if (this->vaultmplog != NULL)
    {
        this->Print((char*) "-----------------------------------------------------------------------------------------------------", false);
        this->Print((char*) "END OF LOG", false);

        fclose(vaultmplog);
    }
}

void Debug::GetTimeFormat(char* buf, int size, bool file)
{
    time_t ltime;
    ltime = time(NULL);

    tm* local = localtime(&ltime);
    char timeformat[32];
    ZeroMemory(timeformat, sizeof(timeformat));

    sprintf(timeformat, "%d%c%02d%c%02d%c%02d%c%02d%c%02d", local->tm_year + 1900, '-', local->tm_mon + 1, '-', local->tm_mday, file ? '_' : ' ', local->tm_hour, file ? '-' : ':', local->tm_min, file ? '-' : ':', local->tm_sec);

    if (size > strlen(timeformat))
        strcpy(buf, timeformat);
}

void Debug::Print(char* text, bool timestamp)
{
    while (filemutex);

    filemutex = true;

    if (vaultmplog == NULL) return;

    if (timestamp)
    {
        char buf[32];
        GetTimeFormat(buf, sizeof(buf), false);
        fputc((int) '[', this->vaultmplog);
        fwrite(buf, sizeof(char), strlen(buf), this->vaultmplog);
        fwrite((char*) "] ", sizeof(char), 2, this->vaultmplog);
    }

    fwrite(text, sizeof(char), strlen(text), this->vaultmplog);
    fputc((int) '\n', this->vaultmplog);

    filemutex = false;
}

void Debug::PrintSystem()
{
    while (filemutex);

    filemutex = true;

    FILE* systeminfo = popen("systeminfo", "r");

    if (systeminfo == NULL) return;

    char buf[2048];
    while (fgets(buf, sizeof(buf), systeminfo) != NULL)
        fwrite(buf, sizeof(char), strlen(buf), this->vaultmplog);

    pclose(systeminfo);

    filemutex = false;
}
