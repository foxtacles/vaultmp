/// \file HTTPConnection.h
/// \brief Contains HTTPConnection, used to communicate with web servers
///
/// This file is part of RakNet Copyright 2008 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.jenkinssoftware.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_HTTPConnection==1 && _RAKNET_SUPPORT_TCPInterface==1

#ifndef __HTTP_CONNECTION
#define __HTTP_CONNECTION

#include "Export.h"
#include "RakString.h"
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "DS_Queue.h"

namespace RakNet
{
/// Forward declarations
class TCPInterface;
struct SystemAddress;

/// \brief Use HTTPConnection to communicate with a web server.
/// \details Start an instance of TCPInterface via the Start() command.
/// Instantiate a new instance of HTTPConnection, and associate TCPInterface with the class in the constructor.
/// Use Post() to send commands to the web server, and ProcessDataPacket() to update the connection with packets returned from TCPInterface that have the system address of the web server
/// This class will handle connecting and reconnecting as necessary.
///
/// Note that only one Post() can be handled at a time. 
/// \deprecated, use HTTPConnection2
class RAK_DLL_EXPORT HTTPConnection
{
public:
	// GetInstance() and DestroyInstance(instance*)
	STATIC_FACTORY_DECLARATIONS(HTTPConnection)

    /// Returns a HTTP object associated with this tcp connection
    HTTPConnection();
    virtual ~HTTPConnection();

	/// \pre tcp should already be started
	void Init(TCPInterface *_tcp, const char *host, unsigned short port=80);

    /// Submit data to the HTTP server
    /// HTTP only allows one request at a time per connection
    ///
	/// \pre IsBusy()==false
    /// \param path the path on the remote server you want to POST to. For example "index.html"
    /// \param data A NULL terminated string to submit to the server
	/// \param contentType "Content-Type:" passed to post.
    void Post(const char *path, const char *data, const char *_contentType="application/x-www-form-urlencoded");

	/// Get a file from a webserver
	/// \param path the path on the remote server you want to GET from. For example "index.html"
	void Get(const char *path);
    
	/// Is there a Read result ready?
	bool HasRead(void) const;

    /// Get one result from the server
	/// \pre HasResult must return true
    RakNet::RakString Read(void);

	/// Call periodically to do time-based updates
	void Update(void);

	/// Returns the address of the server we are connected to
	SystemAddress GetServerAddress(void) const;

	/// Process an HTTP data packet returned from TCPInterface
	/// Returns true when we have gotten all the data from the HTTP server.
    /// If this returns true then it's safe to Post() another request
	/// Deallocate the packet as usual via TCPInterface
    /// \param packet NULL or a packet associated with our host and port
   void ProcessTCPPacket(Packet *packet);

    /// Results of HTTP requests.  Standard response codes are < 999
    /// ( define HTTP codes and our internal codes as needed )
    enum ResponseCodes { NoBody=1001, OK=200, Deleted=1002 };

	HTTPConnection& operator=(const HTTPConnection& rhs){(void) rhs; return *this;}
   
    /// Encapsulates a raw HTTP response and response code
    struct BadResponse
    {
    public:
		BadResponse() {code=0;}
        
        BadResponse(const unsigned char *_data, int _code)
            : data((const char *)_data), code(_code) {}
        
        BadResponse(const char *_data, int _code)
            : data(_data), code(_code) {}

		operator int () const { return code; }

		RakNet::RakString data;
		int code;  // ResponseCodes
    };

    /// Queued events of failed exchanges with the HTTP server
    bool HasBadResponse(int *code, RakNet::RakString *data);

	/// Returns false if the connection is not doing anything else
	bool IsBusy(void) const;

	/// \internal
	int GetState(void) const;

	struct OutgoingCommand
	{
		RakNet::RakString remotePath;
		RakNet::RakString data;
		RakNet::RakString contentType;
		bool isPost;
	};

	 DataStructures::Queue<OutgoingCommand> outgoingCommand;
	 OutgoingCommand currentProcessingCommand;

private:
    SystemAddress server;
    TCPInterface *tcp;
	RakNet::RakString host;
	unsigned short port;
	DataStructures::Queue<BadResponse> badResponses;

	enum ConnectionState
	{
		CS_NONE,
		CS_DISCONNECTING,
		CS_CONNECTING,
		CS_CONNECTED,
		CS_PROCESSING,
	} connectionState;

	RakNet::RakString incomingData;
	DataStructures::Queue<RakNet::RakString> results;

	void CloseConnection();
	
	/*
	enum { RAK_HTTP_INITIAL,
		RAK_HTTP_STARTING,
		RAK_HTTP_CONNECTING,
		RAK_HTTP_ESTABLISHED,
		RAK_HTTP_REQUEST_SENT,
		RAK_HTTP_IDLE } state;

    RakNet::RakString outgoing, incoming, path, contentType;
    void Process(Packet *packet); // the workhorse
    
    // this helps check the various status lists in TCPInterface
	typedef SystemAddress (TCPInterface::*StatusCheckFunction)(void);
	bool InList(StatusCheckFunction func);
	*/

};

} // namespace RakNet

#endif

#endif // _RAKNET_SUPPORT_*
