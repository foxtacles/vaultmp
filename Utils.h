#ifndef UTILS_H
#define UTILS_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <string>

using namespace std;

class Utils
{

private:
    static unsigned int updateCRC32(unsigned char ch, unsigned int crc);

    Utils();

public:
    static void timestamp();
    static string LongToHex(unsigned int value);
    static unsigned int crc32buf(char* buf, size_t len);
    static bool crc32file(char* name, unsigned int* crc, long* charcnt);

};

#endif
