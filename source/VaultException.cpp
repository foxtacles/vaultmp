#include "VaultException.h"

#ifdef VAULTMP_DEBUG
Debug* VaultException::debug;
#endif

VaultException::VaultException(string error)
{
	this->error = error;

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->Print(error.c_str(), true);

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

	if (debug)
		debug->Print(text, true);

#endif
}

#ifdef VAULTMP_DEBUG
void VaultException::SetDebugHandler(Debug* debug)
{
	VaultException::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to VaultException class", true);
}

void VaultException::FinalizeDebug()
{
	if (debug)
		delete debug;

	debug = nullptr;
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

const char* VaultException::what() const throw()
{
	return this->error.c_str();
}
