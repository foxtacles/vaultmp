/// \file HTTPConnection2.h
/// \brief Contains HTTPConnection2, used to communicate with web servers
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
#if _RAKNET_SUPPORT_HTTPConnection2==1 && _RAKNET_SUPPORT_TCPInterface==1

#ifndef __HTTP_CONNECTION_2
#define __HTTP_CONNECTION_2

#include "Export.h"
#include "RakString.h"
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "DS_List.h"
#include "DS_Queue.h"
#include "PluginInterface2.h"
#include "SimpleMutex.h"

namespace RakNet
{
/// Forward declarations
class TCPInterface;
struct SystemAddress;

/// \brief Use HTTPConnection2 to communicate with a web server.
/// \details Start an instance of TCPInterface via the Start() command.
/// This class will handle connecting to transmit a request
class RAK_DLL_EXPORT HTTPConnection2 : public PluginInterface2
{
public:
	// GetInstance() and DestroyInstance(instance*)
	STATIC_FACTORY_DECLARATIONS(HTTPConnection2)

    HTTPConnection2();
    virtual ~HTTPConnection2();

	/// \brief Connect to, then transmit a request to a TCP based server
	/// \param[in] tcp An instance of TCPInterface that previously had TCPInterface::Start() called
	/// \param[in] stringToTransmit What string to transmit. See RakString::FormatForPOST(), RakString::FormatForGET(), RakString::FormatForDELETE()
	/// \param[in] host The IP address to connect to
	/// \param[in] port The port to connect to
	/// \param[in] useSSL If to use SSL to connect. OPEN_SSL_CLIENT_SUPPORT must be defined to 1 in RakNetDefines.h or RakNetDefinesOverrides.h
	/// \param[in] ipVersion 4 for IPV4, 6 for IPV6
	/// \param[in] useAddress Assume we are connected to this address and send to it, rather than do a lookup
	/// \return false if host is not a valid IP address or domain name
	bool TransmitRequest(const char* stringToTransmit, const char* host, unsigned short port=80, bool useSSL=false, int ipVersion=4, SystemAddress useAddress=UNASSIGNED_SYSTEM_ADDRESS);

	/// \brief Check for and return a response from a prior call to TransmitRequest()
	/// As TCP is stream based, you may get a webserver reply over several calls to TCPInterface::Receive()
	/// HTTPConnection2 will store Packet::data and return the response to you either when the connection to the webserver is lost, or enough data has been received()
	/// This will only potentially return true after a call to ProcessTCPPacket() or OnLostConnection()
	/// \param[out] stringTransmitted The original string transmitted
	/// \param[out] hostTransmitted The parameter of the same name passed to TransmitRequest()
	/// \param[out] responseReceived The response, if any
	/// \param[out] hostReceived The SystemAddress from ProcessTCPPacket() or OnLostConnection()
	/// \param[out] contentOffset The offset from the start of responseReceived to the data body. Equivalent to searching for \r\n\r\n in responseReceived.
	/// \return true if there was a response. false if not.
	bool GetResponse( RakString &stringTransmitted, RakString &hostTransmitted, RakString &responseReceived, SystemAddress &hostReceived, int &contentOffset );

	/// \brief Return if any requests are pending
	bool IsBusy(void) const;

	/// \brief Return if any requests are waiting to be read by the user
	bool HasResponse(void) const;

	struct Request
	{
		RakString stringToTransmit;
		RakString stringReceived;
		RakString host;
		SystemAddress hostEstimatedAddress;
		SystemAddress hostCompletedAddress;
		unsigned short port;
		bool useSSL;
		int contentOffset;
		int contentLength;
		int ipVersion;
		bool chunked;
		size_t thisChunkSize;
		size_t bytesReadForThisChunk;
	};

	/// \internal
	virtual PluginReceiveResult OnReceive(Packet *packet);
	virtual void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason );
	virtual void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming);
	virtual void OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason);

protected:

	bool IsConnected(SystemAddress sa);
	void SendRequest(Request *request);
	void RemovePendingRequest(SystemAddress sa);
	void SendNextPendingRequest(void);
	void SendPendingRequestToConnectedSystem(SystemAddress sa);

	DataStructures::Queue<Request*> pendingRequests;
	DataStructures::List<Request*> sentRequests;
	DataStructures::List<Request*> completedRequests;

	SimpleMutex pendingRequestsMutex, sentRequestsMutex, completedRequestsMutex;

};

} // namespace RakNet

#endif

#endif // _RAKNET_SUPPORT_*
