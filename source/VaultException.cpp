#include "VaultException.hpp"

#ifdef VAULTMP_DEBUG
#include <sstream>
#include "dbg/stack.hpp"
#endif

#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <cstring>
#include <cstdarg>

using namespace std;

#ifdef VAULTMP_DEBUG
DebugInput<VaultException> VaultException::debug;
#endif

VaultException::VaultException(const string& error) : error(error)
{
#ifdef VAULTMP_DEBUG
	debug.print(error.c_str());
#endif
}

VaultException::VaultException(const char* format, ...)
{
	char text[256];
	ZeroMemory(text, sizeof(text));

	va_list args;
	va_start(args, format);
	vsnprintf(text, sizeof(text), format, args);
	va_end(args);

	this->error = string(text);

#ifdef VAULTMP_DEBUG
	debug.print(error.c_str());
#endif
}

#ifdef VAULTMP_DEBUG
void VaultException::stacktrace_()
{
	dbg::stack st;
	ostringstream stream;

	for (const auto& frame : st)
		stream << frame << "\n";

	debug.print(stream.str().c_str());
}
#endif

void VaultException::Message()
{
#ifdef __WIN32__
	MessageBox(nullptr, error.c_str(), "Fatal error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_TASKMODAL);
#endif
}

void VaultException::Console()
{
	printf("%s\n", error.c_str());
}

VaultException& VaultException::stacktrace()
{
#ifdef VAULTMP_DEBUG
	try
	{
		stacktrace_();
	}
	catch (...) {}
#endif
	return *this;
}

const char* VaultException::what() const throw()
{
	return this->error.c_str();
}
