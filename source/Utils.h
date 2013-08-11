#ifndef UTILS_H
#define UTILS_H

#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <string>

namespace Utils
{
		template<std::size_t N>
		constexpr unsigned int hash(const char(&str)[N], std::size_t I = N) {
			return (I == 1 ? ((2166136261u ^ str[0]) * 16777619u) : ((hash(str, I - 1) ^ str[I - 1]) * 16777619u));
		}

		void timestamp();
		int progress_func(double TotalToDownload, double NowDownloaded);
		bool DoubleCompare(double a, double b, double epsilon);
		std::string toString(signed int value);
		std::string toString(unsigned int value);
		std::string toString(unsigned char value);
		std::string toString(double value);
		std::string toString(unsigned long long value);
		std::string str_replace(const std::string& source, const char* find, const char* replace);
		std::string& RemoveExtension(std::string& file);
		const char* FileOnly(const char* path);
		unsigned int FileLength(const char* file);
		unsigned int crc32buf(char* buf, size_t len);
		unsigned int updateCRC32(unsigned char ch, unsigned int crc);
		bool crc32file(const char* name, unsigned int* crc);

#ifdef __WIN32__
		BOOL GenerateChecksum(const std::string& szFilename, DWORD& dwExistingChecksum, DWORD& dwChecksum);
#endif
}

#endif
