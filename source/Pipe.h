#include <winsock2.h>
#include <string>
#include <vector>

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
		unsigned int Send(const unsigned char* stream);
		void Receive(unsigned char* stream);
		virtual bool ConnectToServer() = 0;
};

class PipeServer : public Pipe
{
	public:
		bool CreateServer();
		bool ConnectToServer();
};

class PipeClient : public Pipe
{
	public:
		bool ConnectToServer();
};
