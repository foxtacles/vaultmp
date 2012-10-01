#include <winsock2.h>
#include <string>
#include <vector>

using namespace std;

class Pipe
{
	private:
		Pipe(const Pipe&) = delete;
		Pipe& operator=(const Pipe&) = delete;

	protected:
		Pipe();
		virtual ~Pipe();

		string name;
		unsigned int size;
		HANDLE pipe;

	public:
		void SetPipeAttributes(const string& name, unsigned int size);
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
