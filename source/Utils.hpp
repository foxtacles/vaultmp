#ifndef UTILS_H
#define UTILS_H

#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <string>
#include <type_traits>

namespace Utils
{
		template<size_t N>
		constexpr unsigned int hash(const char(&str)[N], size_t I = N) {
			return (I == 1 ? ((2166136261u ^ str[0]) * 16777619u) : ((hash(str, I - 1) ^ str[I - 1]) * 16777619u));
		}

		inline unsigned int hash(const char* str, std::size_t I) {
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

		template<typename F, typename T, typename E = void> struct is_static_castable : std::false_type {};
		template<typename F, typename T> struct is_static_castable<F, T, typename std::conditional<true, void, decltype(static_cast<T>(std::declval<F>()))>::type> : std::true_type {};

		template<typename T, typename F> inline static typename std::enable_if<is_static_castable<F*, T*>::value, T*>::type static_or_dynamic_cast(F* from) { return static_cast<T*>(from); }
		template<typename T, typename F> inline static typename std::enable_if<!is_static_castable<F*, T*>::value, T*>::type static_or_dynamic_cast(F* from) { return dynamic_cast<T*>(from); }

#ifdef __WIN32__
		BOOL GenerateChecksum(const std::string& szFilename, DWORD& dwExistingChecksum, DWORD& dwChecksum);
#endif
}

#endif
