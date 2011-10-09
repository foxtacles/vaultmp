#include "Pipe.h"

using namespace std;

Pipe::Pipe()
{
	this->name = "pipe";
	this->size = 0;
	this->pipe = NULL;
}

Pipe::~Pipe()
{
	CloseHandle( this->pipe );
}

void Pipe::SetPipeAttributes( string name, unsigned int size )
{
	this->name = "\\\\.\\pipe\\" + name;
	this->size = size;
}

unsigned int Pipe::Send( char* stream )
{
	DWORD dwActuallyWritten;

	char buffer[this->size];
	ZeroMemory( buffer, sizeof( buffer ) );

	memcpy( buffer, stream, sizeof( buffer ) );

	if ( !WriteFile( this->pipe, &buffer, this->size, &dwActuallyWritten, NULL ) )
		return 0;

	else
		return dwActuallyWritten;
}

unsigned int Pipe::Send( string stream )
{
	char c_stream[this->size];
	ZeroMemory( c_stream, sizeof( c_stream ) );
	strncpy( c_stream, stream.c_str(), sizeof( c_stream ) - 1 );
	return Send( c_stream );
}

void Pipe::Receive( char* stream )
{
	char buffer[this->size];
	ZeroMemory( buffer, sizeof( buffer ) );

	DWORD dwActuallyRead;

	if ( !ReadFile( this->pipe, &buffer, this->size, &dwActuallyRead, NULL ) )
		return;

	memcpy( stream, buffer, sizeof( buffer ) );
}

bool PipeServer::CreateServer()
{
	if ( this->size == 0 )
		return false;

	this->pipe = CreateNamedPipe( this->name.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, this->size, this->size, 0, NULL );

	if ( this->pipe == INVALID_HANDLE_VALUE )
		return false;

	return true;
}

bool PipeServer::ConnectToServer()
{
	if ( this->pipe == INVALID_HANDLE_VALUE )
		return false;

	if ( ConnectNamedPipe( this->pipe, NULL ) )
		return true;

	return false;
}

bool PipeClient::ConnectToServer()
{
	if ( this->name == "pipe" )
		return false;

	while ( true )
	{
		this->pipe = CreateFile( this->name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

		if ( this->pipe != INVALID_HANDLE_VALUE )
			break;

		if ( GetLastError() != ERROR_PIPE_BUSY )
			return false;

		if ( !WaitNamedPipe( this->name.c_str(), 5000 ) )
			return false;
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE;

	if ( !SetNamedPipeHandleState( this->pipe, &dwMode, NULL, NULL ) )
		return false;

	return true;
}
