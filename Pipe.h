#include <winsock2.h>
#include <string>

using namespace std;

class Pipe
{

	private:
		Pipe(const Pipe&);
		Pipe& operator=(const Pipe&);

	protected:
		Pipe();
		virtual ~Pipe();

		string name;
		unsigned int size;
		HANDLE pipe;

	public:
		void SetPipeAttributes(string name, unsigned int size);
		unsigned int Send(unsigned char* stream);
		unsigned int Send(string stream);
		void Receive(unsigned char* stream);
		virtual bool ConnectToServer() = 0;

};

class PipeServer : public Pipe
{

	private:
		PipeServer& operator=(const PipeServer&);

	public:
		bool CreateServer();
		bool ConnectToServer();

};

class PipeClient : public Pipe
{

	public:
		bool ConnectToServer();

};
