#ifndef PIPE_H
#define PIPE_H

#include <winsock2.h>
#include <string>

class Pipe
{
	private:
		Pipe(const Pipe&) = delete;
		Pipe& operator=(const Pipe&) = delete;

	protected:
		Pipe();
		virtual ~Pipe();

		std::string name;
		unsigned int size;
		HANDLE pipe;

	public:
		void SetPipeAttributes(const std::string& name, unsigned int size);
		bool Send(const unsigned char* stream);
		bool Receive(unsigned char* stream);
		virtual bool ConnectToServer() = 0;
};

class PipeServer : public Pipe
{
	public:
		bool CreateServer();
		bool ConnectToServer();
		bool Disconnect();
};

class PipeClient : public Pipe
{
	public:
		bool ConnectToServer();
};

#endif
