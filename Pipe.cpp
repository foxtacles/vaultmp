#include "Pipe.h"

using namespace pipe;
using namespace std;

PipeServer::PipeServer()
{
    this->pipeName = "pipe";
    this->size = 0;
    this->pipe = NULL;
}

void PipeServer::SetPipeAttributes(string pName, unsigned int pSize)
{
    this->pipeName = "\\\\.\\pipe\\" + pName;
    this->size = pSize;
}

bool PipeServer::CreateServer()
{
    if (this->size == 0)
        return false;
        
    this->pipe = CreateNamedPipe(this->pipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, this->size, this->size, 0, NULL);
    
    if (this->pipe == INVALID_HANDLE_VALUE)
        return false;
        
    return true;
}

bool PipeServer::ConnectToServer()
{
    if (this->pipe == INVALID_HANDLE_VALUE)
        return false;
        
    if (ConnectNamedPipe(this->pipe, NULL))
        return true;
        
    return false;
}

unsigned int PipeServer::Send(string* strSend)
{
    DWORD dwActuallyWritten;
    
    if (!WriteFile(this->pipe, strSend->c_str(), strSend->size(), &dwActuallyWritten, NULL))
        return 0;
    else
        return dwActuallyWritten;
}

string PipeServer::Recv()
{
    if (this->size == 0)
        return "-.-";

    char buffer[this->size];
    
    DWORD dwActuallyRead;
    
    if (!ReadFile(this->pipe, &buffer, this->size, &dwActuallyRead, NULL))
        return "-.-";

    string str = buffer;

    return str;
}

PipeClient::PipeClient()
{
    this->pipeName = "pipe";
    this->size = 0;
    this->pipe = NULL;
}

void PipeClient::SetPipeAttributes(string pName, unsigned int pSize)
{
    this->pipeName = "\\\\.\\pipe\\" + pName;
    this->size = pSize;
}

bool PipeClient::ConnectToServer()
{
    if (this->pipeName == "pipe")
        return false;

    while (true)
    {
        this->pipe = CreateFile(this->pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        
        if (this->pipe != INVALID_HANDLE_VALUE)
            break;

        if (GetLastError() != ERROR_PIPE_BUSY)
            return false;

        if (!WaitNamedPipe(this->pipeName.c_str(), 5000))
            return false;
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE;
    
    if (!SetNamedPipeHandleState(this->pipe, &dwMode, NULL, NULL))
        return false;

    return true;
}

unsigned int PipeClient::Send(string* strSend)
{
    DWORD dwActuallyWritten;
    
    if (!WriteFile(this->pipe, strSend->c_str(), strSend->size(), &dwActuallyWritten, NULL))
        return 0;
    else
        return dwActuallyWritten;
}

string PipeClient::Recv()
{
    if (this->size == 0)
        return "-.-";

    char buffer[this->size];
    
    DWORD dwActuallyRead;
    
    if (!ReadFile(this->pipe, &buffer, this->size, &dwActuallyRead, NULL))
        return "-.-";

    string str = buffer;

    return str;
}
