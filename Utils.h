#ifndef UTILS_H
#define UTILS_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cstring>

#include <string>

using namespace std;

class Utils
{

private:
    static unsigned int updateCRC32(unsigned char ch, unsigned int crc);

    Utils();

public:
    static void timestamp();
    static bool DoubleCompare(double a, double b, double epsilon);
    static string LongToHex(unsigned int value);
    static string& RemoveExtension(string& file);
    static const char* FileOnly(const char* path);
    static unsigned int FileLength(const char* file);
    static unsigned int crc32buf(char* buf, size_t len);
    static bool crc32file(char* name, unsigned int* crc);

};

#endif
