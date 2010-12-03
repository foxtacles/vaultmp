#include <windows.h>
#include <string>

namespace pipe
{
    class PipeServer
    {
    private:
        std::string pipeName;
        unsigned int size;
        HANDLE pipe;
    public:
        PipeServer();
        ~PipeServer();
        void SetPipeAttributes(std::string pName, unsigned int pSize);
        bool CreateServer();
        bool ConnectToServer();
        unsigned int Send(std::string* strSend);
        std::string Recv();
    };

    class PipeClient
    {
    private:
        std::string pipeName;
        unsigned int size;
        HANDLE pipe;
    public:
        PipeClient();
        ~PipeClient();
        void SetPipeAttributes(std::string pName, unsigned int pSize);
        bool ConnectToServer();
        unsigned int Send(std::string* strSend);
        std::string Recv();
    };
};
