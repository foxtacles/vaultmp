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
#include <fstream>

#include "vaultmp.h"
#include "CriticalSection.h"

template<typename T>
struct DebugInput;

class Debug : private CriticalSection
{
		template<typename T>
		friend struct DebugInput;

	private:
		Debug(const char* module);

		std::fstream vaultmplog;
		static std::unique_ptr<Debug> debug;

		static void GetTimeFormat(char* buf, unsigned int size, bool file);

		template <typename T, typename... Args>
		void Note(T&& arg, Args&&... args)
		{
			vaultmplog << std::forward<T>(arg);
			Note(std::forward<Args>(args)...);
		}

		void Note()
		{
			vaultmplog << std::endl;
		}

		template <typename... Args>
		void Print(Args&&... args)
		{
			char buf[32];
			GetTimeFormat(buf, sizeof(buf), false);
			vaultmplog << "[" << buf << "] ";

			Note(std::forward<Args>(args)...);
		}

		Debug(const Debug&) = delete;
		Debug& operator=(const Debug&) = delete;

	public:
		~Debug();

		static void SetDebugHandler(const char* module);
};

template<typename T>
struct DebugInput
{
	public:
		template <typename... Args>
		const DebugInput& note(Args&&... args) const
		{
			Debug::debug->Note(std::forward<Args>(args)...);
			return *this;
		}

		template <typename... Args>
		const DebugInput& print(Args&&... args) const
		{
			Debug::debug->Print(std::forward<Args>(args)...);
			return *this;
		}
};

#endif
