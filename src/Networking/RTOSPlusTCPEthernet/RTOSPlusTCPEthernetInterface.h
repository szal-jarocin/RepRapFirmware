
#ifndef RTOSPLUSTCPETHERNET_H
#define RTOSPLUSTCPETHERNET_H

#include "Networking/NetworkInterface.h"
#include "NetworkDefs.h"
#include "MessageType.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

class NetworkResponder;
class HttpResponder;
class FtpResponder;
class TelnetResponder;
class RTOSPlusTCPEthernetSocket;


#if __LPC17xx__
const size_t NumHttpSockets = 1;				// sockets 0-3 are for HTTP
#else
const size_t NumHttpSockets = 4;                // sockets 0-3 are for HTTP
#endif
const SocketNumber FtpSocketNumber = 4;
const SocketNumber FtpDataSocketNumber = 5;		// TODO can we allocate this dynamically when required, to allow more http sockets most of the time?
const SocketNumber TelnetSocketNumber = 6;
const size_t NumRTOSPlusTCPEthernetTcpSockets = 7;
const SocketNumber DhcpSocketNumber = 7;		// TODO can we allocate this dynamically when required, to allow more http sockets most of the time?

class Platform;

// The main network class that drives the network.
class RTOSPlusTCPEthernetInterface : public NetworkInterface
{
public:
	RTOSPlusTCPEthernetInterface(Platform& p);

	void Init() override;
	void Activate() override;
	void Exit() override;
	void Spin(bool full) override;
	void Diagnostics(MessageType mtype) override;

	GCodeResult EnableInterface(int mode, const StringRef& ssid, const StringRef& reply) override;			// enable or disable the network
	GCodeResult EnableProtocol(NetworkProtocol protocol, int port, int secure, const StringRef& reply) override;
	GCodeResult DisableProtocol(NetworkProtocol protocol, const StringRef& reply) override;
	GCodeResult ReportProtocols(const StringRef& reply) const override;

	GCodeResult GetNetworkState(const StringRef& reply) override;
	int EnableState() const override;
	bool InNetworkStack() const override { return false; }
	bool IsWiFiInterface() const override { return false; }

	void UpdateHostname(const char *name) override { }
	const uint8_t *GetIPAddress() const override { return ipAddress; }
	void SetMacAddress(const uint8_t mac[]) override;
	const uint8_t *GetMacAddress() const override { return macAddress; }

	void OpenDataPort(Port port) override;
	void TerminateDataPort() override;
    
    
    
private:
	enum class NetworkState
	{
		disabled,					// Network disabled
		enabled,					// Network enabled but not started yet
		establishingLink,			// starting up, waiting for link
		obtainingIP,				// link established, waiting for DHCP
		connected,					// just established a connection
		active						// network running
	};

	void Start();
	void Stop();
	void InitSockets();
	void TerminateSockets();

	void StartProtocol(NetworkProtocol protocol)
	pre(protocol < NumProtocols);

	void ShutdownProtocol(NetworkProtocol protocol)
	pre(protocol < NumProtocols);

	void ReportOneProtocol(NetworkProtocol protocol, const StringRef& reply) const
	pre(protocol < NumProtocols);

	void SetIPAddress(const uint8_t p_ipAddress[], const uint8_t p_netmask[], const uint8_t p_gateway[]);
	Platform& platform;
	uint32_t lastTickMillis;

	RTOSPlusTCPEthernetSocket *sockets[NumRTOSPlusTCPEthernetTcpSockets];
	size_t nextSocketToPoll;						// next TCP socket number to poll for read/write operations

	Port portNumbers[NumProtocols];					// port number used for each protocol
	bool protocolEnabled[NumProtocols];				// whether each protocol is enabled

    
    void ProcessIPApplication( eIPCallbackEvent_t eNetworkEvent );
    eDHCPCallbackAnswer_t ProcessDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress );
    const char *ProcessApplicationHostnameHook();
    
    friend void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent );
    friend eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress );
    friend const char *pcApplicationHostnameHook( void );
    
    
	NetworkState state;
	bool activated;
	bool usingDhcp;

	uint8_t ipAddress[4];
	uint8_t netmask[4];
	uint8_t gateway[4];
    uint8_t dnsServerAddress[4];
	uint8_t macAddress[6];
};

#endif //RTOSPLUSTCPETHERNET_H
