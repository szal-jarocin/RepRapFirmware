/*
 * RTOSPlusTCPEthernetInterface.cpp
 *
 * SD: RTOSPlusTCPEthernet adapted from W5500Interface class to work with FreeRTOS+TCP
 
 */

#include "RTOSPlusTCPEthernetInterface.h"
#include "RTOSPlusTCPEthernetSocket.h"
#include "NetworkBuffer.h"
#include "Platform.h"
#include "RepRap.h"
#include "HttpResponder.h"
#include "FtpResponder.h"
#include "TelnetResponder.h"
#include "General/IP4String.h"

#include <FreeRTOS.h>
#include "task.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"

#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"
#include "RTOSIface.h"

constexpr size_t TcpPlusStackWords = ( uint16_t ) ipconfigIP_TASK_STACK_SIZE_WORDS; // needs to be aroud 240 when debugging with debugPrintf
static Task<TcpPlusStackWords> tcpPlusTask;

constexpr size_t EmacStackWords = 74;/*170*/ // needs to be bigger (>=170) if using debugPrinf for testing
static Task<EmacStackWords> emacTask;



//convert the 32bit ipaddress to array

inline void convert32bitIPAddress(uint32_t &ulIPAddress, uint8_t ip[])
{
    ip[0] = ((unsigned) ((ulIPAddress) & 0xffUL));
    ip[1] = ((unsigned) (((ulIPAddress) >> 8 ) & 0xffUL));
    ip[2] = ((unsigned) (((ulIPAddress) >> 16 ) & 0xffUL));
    ip[3] = ((unsigned) ((ulIPAddress) >> 24 ));

    
}

RTOSPlusTCPEthernetInterface *rtosTCPEtherInterfacePtr; //pointer to the clas instance so we can call from the c hooks

extern "C" {
    
    /* Use by the pseudo random number generator. in +TCP */
    static UBaseType_t ulNextRand;


    //SD:: use the RFF tasks management to create tasks needed for +tcp
    TaskHandle_t RRfInitialiseIPTask(TaskFunction_t pxTaskCode) {
        //original call in +tcp
        //xReturn = xTaskCreate( prvIPTask, "IP-task", ( uint16_t ) ipconfigIP_TASK_STACK_SIZE_WORDS, NULL, ( UBaseType_t ) ipconfigIP_TASK_PRIORITY, &xIPTaskHandle );

        tcpPlusTask.Create(pxTaskCode, "IP-task", nullptr, ( UBaseType_t ) /*ipconfigIP_TASK_PRIORITY*/TaskBase::TcpPriority);
        return tcpPlusTask.GetHandle();
    }
    
    TaskHandle_t RRfInitialiseEMACTask(TaskFunction_t pxTaskCode) {
        //Original EMAC task creation in NetworkInterface.c (portable lpc17xx)
        //xReturn = xTaskCreate( prvEMACHandlerTask, "EMAC", nwRX_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &xRxHanderTask );

        //TODO::Determine priority to run EMAC at
        emacTask.Create(pxTaskCode, "EMAC", nullptr, /*configMAX_PRIORITIES - 1*/ TaskBase::TcpPriority);
        //FreeRTOS_debug_printf( ( "RRfInitialiseIPTask: Creating Task for EMAC \n" ) );
        return emacTask.GetHandle();
    }
    
} // end extern "C"




//TODO:: Currently NetworkInferface.c expects ucMACAddress to be externally defined....
//       IP Init doesnt seem to pass on the mac address to the portable layer
uint8_t ucMACAddress[ 6 ];





//FreeRTOS +TCP "C" application Hooks

 /* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect events are only received if implemented in the MAC driver. */
extern "C" void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    (rtosTCPEtherInterfacePtr)->ProcessIPApplication(eNetworkEvent); //call the c++ class to handle the callback
}

//// RTOS+TCP DHCP Hook.... this allows us to control over the DHCP process.
extern "C" eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress )
{
    return (rtosTCPEtherInterfacePtr)->ProcessDHCPHook(eDHCPPhase, ulIPAddress);
}

extern "C" const char *pcApplicationHostnameHook( void )
{
    return (rtosTCPEtherInterfacePtr)->ProcessApplicationHostnameHook();
}


//ipconfigRAND32
//Random required for various FreeRTOS+TCP functions, sockets etc
// from example
extern "C" UBaseType_t uxRand( void )
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

//**********************


RTOSPlusTCPEthernetInterface::RTOSPlusTCPEthernetInterface(Platform& p)
	: platform(p), lastTickMillis(0), state(NetworkState::disabled), activated(false)
{
    //setup our pointer to access our class methods from the +TCP "C" callbacks
    rtosTCPEtherInterfacePtr = this;
    
	// Create the sockets
	for (RTOSPlusTCPEthernetSocket*& skt : sockets)
	{
		skt = new RTOSPlusTCPEthernetSocket(this);
	}

	for (size_t i = 0; i < NumProtocols; ++i)
	{
		portNumbers[i] = DefaultPortNumbers[i];
		protocolEnabled[i] = (i == HttpProtocol);
	}
}




void RTOSPlusTCPEthernetInterface::Init()
{
	interfaceMutex.Create("RTOSPlusTCPEthernet");
	SetIPAddress(DefaultIpAddress, DefaultNetMask, DefaultGateway);
	memcpy(macAddress, platform.GetDefaultMacAddress(), sizeof(macAddress));
}




GCodeResult RTOSPlusTCPEthernetInterface::EnableProtocol(NetworkProtocol protocol, int port, int secure, const StringRef& reply)
{
	if (secure != 0 || secure != -1)
	{
		reply.copy("this firmware does not support TLS");
		return GCodeResult::error;
	}

	if (protocol < NumProtocols)
	{
		MutexLocker lock(interfaceMutex);

		const Port portToUse = (port < 0) ? DefaultPortNumbers[protocol] : port;
		if (portToUse != portNumbers[protocol] && state == NetworkState::active)
		{
			// We need to shut down and restart the protocol if it is active because the port number has changed
			ShutdownProtocol(protocol);
			protocolEnabled[protocol] = false;
		}
		portNumbers[protocol] = portToUse;
		if (!protocolEnabled[protocol])
		{
			protocolEnabled[protocol] = true;
			if (state == NetworkState::active)
			{
				StartProtocol(protocol);
                

                //SD::MDS is done by +TCP (if enabled in config)
//                if (state == NetworkState::active)
//                {
//                    DoMdnsAnnounce();
//                }
			}
		}
		ReportOneProtocol(protocol, reply);
		return GCodeResult::ok;
	}

	reply.copy("Invalid protocol parameter");
	return GCodeResult::error;
}

GCodeResult RTOSPlusTCPEthernetInterface::DisableProtocol(NetworkProtocol protocol, const StringRef& reply)
{
	if (protocol < NumProtocols)
	{
		MutexLocker lock(interfaceMutex);

		if (state == NetworkState::active)
		{
			ShutdownProtocol(protocol);
		}
		protocolEnabled[protocol] = false;
		ReportOneProtocol(protocol, reply);
		return GCodeResult::ok;
	}
	reply.copy("Invalid protocol parameter");
	return GCodeResult::error;
}

void RTOSPlusTCPEthernetInterface::StartProtocol(NetworkProtocol protocol)
{
	MutexLocker lock(interfaceMutex);
	switch(protocol)
	{
	case HttpProtocol:

		for (SocketNumber skt = 0; skt < NumHttpSockets; ++skt)
		{
			sockets[skt]->Init(skt, portNumbers[protocol], protocol);
		}
		break;

	case FtpProtocol:
		sockets[FtpSocketNumber]->Init(FtpSocketNumber, portNumbers[protocol], protocol);
		break;

	case TelnetProtocol:
		sockets[TelnetSocketNumber]->Init(TelnetSocketNumber, portNumbers[protocol], protocol);
		break;

	default:
		break;
	}
}

void RTOSPlusTCPEthernetInterface::ShutdownProtocol(NetworkProtocol protocol)
{
	MutexLocker lock(interfaceMutex);

    switch(protocol)
	{
	case HttpProtocol:
		for (SocketNumber skt = 0; skt < NumHttpSockets; ++skt)
		{
			sockets[skt]->TerminateAndDisable();
		}
		break;

	case FtpProtocol:
		sockets[FtpSocketNumber]->TerminateAndDisable();
		sockets[FtpDataSocketNumber]->TerminateAndDisable();
		break;

	case TelnetProtocol:
		sockets[TelnetSocketNumber]->TerminateAndDisable();
		break;

	default:
		break;
	}
}

// Report the protocols and ports in use
GCodeResult RTOSPlusTCPEthernetInterface::ReportProtocols(const StringRef& reply) const
{
    reply.Clear();
	for (size_t i = 0; i < NumProtocols; ++i)
	{
		if (i != 0)
		{
			reply.cat('\n');
		}
		ReportOneProtocol(i, reply);
	}
	return GCodeResult::ok;
}

void RTOSPlusTCPEthernetInterface::ReportOneProtocol(NetworkProtocol protocol, const StringRef& reply) const
{
	if (protocolEnabled[protocol])
	{
		reply.catf("%s is enabled on port %u", ProtocolNames[protocol], portNumbers[protocol]);
	}
	else
	{
		reply.catf("%s is disabled", ProtocolNames[protocol]);
	}
}

// This is called at the end of config.g processing.
// Start the network if it was enabled

void RTOSPlusTCPEthernetInterface::Activate()
{
	if (!activated)
	{
		activated = true;
        
        
		if (state == NetworkState::enabled)
		{
			Start();
            
		}
		else
		{
			platform.Message(NetworkInfoMessage, "Network disabled.\n");
		}
	}
}

void RTOSPlusTCPEthernetInterface::Exit()
{
	Stop();
}

// Get the network state into the reply buffer, returning true if there is some sort of error
GCodeResult RTOSPlusTCPEthernetInterface::GetNetworkState(const StringRef& reply)
{
	IPAddress config_ip = platform.GetIPAddress();
	const int enableState = EnableState();
	reply.printf("Network is %s, configured IP address: %s, actual IP address: %s",
			(enableState == 0) ? "disabled" : "enabled",
					IP4String(config_ip).c_str(), IP4String(ipAddress).c_str());
	return GCodeResult::ok;
}

// Update the MAC address
void RTOSPlusTCPEthernetInterface::SetMacAddress(const uint8_t mac[])
{
	for (size_t i = 0; i < 6; i++)
	{
		macAddress[i] = mac[i];
	}
}

// Start up the network
void RTOSPlusTCPEthernetInterface::Start()
{
	MutexLocker lock(interfaceMutex);
    
    SetIPAddress(platform.GetIPAddress(), platform.NetMask(), platform.GateWay());

    //time_t xTimeNow;
    //time( &xTimeNow ); // Seed the random number generator.
    //ulNextRand = xTimeNow;

    dnsServerAddress[0]=0;
    dnsServerAddress[1]=0;
    dnsServerAddress[2]=0;
    dnsServerAddress[3]=0;

    //set the global mac var needed for networkinterface.c
    ucMACAddress[0] = macAddress[0];
    ucMACAddress[1] = macAddress[1];
    ucMACAddress[2] = macAddress[2];
    ucMACAddress[3] = macAddress[3];
    ucMACAddress[4] = macAddress[4];
    ucMACAddress[5] = macAddress[5];

    
    uint8_t ip[4], nm[4], gw[4];
    ipAddress.UnpackV4(ip);
    netmask.UnpackV4(nm);
    gateway.UnpackV4(gw);
    
    BaseType_t ret = FreeRTOS_IPInit( ip, nm, gw, dnsServerAddress, macAddress );
    if(ret == pdFALSE){
        FreeRTOS_debug_printf( ( "Failed to start IP") );
        state = NetworkState::disabled;
        return;
    }

    
    state = NetworkState::establishingLink;
}

// Stop the network
void RTOSPlusTCPEthernetInterface::Stop()
{
    
	if (state != NetworkState::disabled)
	{
		MutexLocker lock(interfaceMutex);
        state = NetworkState::disabled;
        //todo: how to stop FreeRTOS+TCP ? (perhaps suspend the IP-Task and EMAC task
        
	}
}

// Main spin loop. If 'full' is true then we are being called from the main spin loop. If false then we are being called during HSMCI idle time.
void RTOSPlusTCPEthernetInterface::Spin(bool full)
{

    switch(state)
	{
	case NetworkState::enabled:
	case NetworkState::disabled:
	default:
		// Nothing to do
		break;

	case NetworkState::establishingLink:
		{
			//MutexLocker lock(interfaceMutex);
            //Nothing to do. FreeRTOS+TCP IP-Task will handle the dhcp etc (controled via the c hooks)
            
		}
		break;

	case NetworkState::obtainingIP:
		if (full)
		{
			//MutexLocker lock(interfaceMutex);

            //nothing to do. Currently handled by the IP-Task

		}
		break;

	case NetworkState::connected:
		if (full)
		{
            InitSockets();
            platform.MessageF(NetworkInfoMessage, "Network running, IP address = %s\n", IP4String(ipAddress).c_str());
            state = NetworkState::active;
		}
		break;

	case NetworkState::active:
		{
			MutexLocker lock(interfaceMutex);

            if(FreeRTOS_IsNetworkUp() == pdTRUE)
            {
                // Poll the next TCP socket
                sockets[nextSocketToPoll]->Poll(full);

                // Move on to the next TCP socket for next time
                ++nextSocketToPoll;
                if (nextSocketToPoll == NumRTOSPlusTCPEthernetTcpSockets)
                {
                    nextSocketToPoll = 0;
                }
            }
            else if (full)
            {
                //Network is Down
                TerminateSockets();
                state = NetworkState::establishingLink;
            }
		}
		break;
	}
}

void RTOSPlusTCPEthernetInterface::Diagnostics(MessageType mtype)
{
	platform.MessageF(mtype, "Interface state: ");
    switch (state){
        case NetworkState::disabled:
            platform.MessageF(mtype, "disabled\n");
            break;
        case NetworkState::enabled:
            platform.MessageF(mtype, "enabled\n");
            break;
        case NetworkState::establishingLink:
            platform.MessageF(mtype, "establishing link\n");
            break;
        case NetworkState::obtainingIP:
            platform.MessageF(mtype, "obtaining IP\n");
            break;
        case NetworkState::connected:
            platform.MessageF(mtype, "connected\n");
            break;
        case NetworkState::active:
            platform.MessageF(mtype, "active\n");
            break;
        default:
            
            break;
    }
}

// Enable or disable the network
GCodeResult RTOSPlusTCPEthernetInterface::EnableInterface(int mode, const StringRef& ssid, const StringRef& reply)
{
	if (!activated)
	{
		state = (mode == 0) ? NetworkState::disabled : NetworkState::enabled;
	}
	else if (mode == 0)
	{
		if (state != NetworkState::disabled)
		{
			Stop();
			platform.Message(NetworkInfoMessage, "Network stopped\n");
		}

	}
	else if (state == NetworkState::disabled)
	{
		state = NetworkState::enabled;
		Start();
	}
    
	return GCodeResult::ok;
}

int RTOSPlusTCPEthernetInterface::EnableState() const
{
	return (state == NetworkState::disabled) ? 0 : 1;
}

//void RTOSPlusTCPEthernetInterface::SetIPAddress(const uint8_t p_ipAddress[], const uint8_t p_netmask[], const uint8_t p_gateway[])
//{
//    memcpy(ipAddress, p_ipAddress, sizeof(ipAddress));
//    memcpy(netmask, p_netmask, sizeof(netmask));
//    memcpy(gateway, p_gateway, sizeof(gateway));
//}

void RTOSPlusTCPEthernetInterface::SetIPAddress(IPAddress p_ip, IPAddress p_netmask, IPAddress p_gateway)
{
    ipAddress = p_ip;
    netmask = p_netmask;
    gateway = p_gateway;
}


void RTOSPlusTCPEthernetInterface::OpenDataPort(Port port)
{
	sockets[FtpDataSocketNumber]->Init(FtpDataSocketNumber, port, FtpDataProtocol);
}

// Close FTP data port and purge associated resources
void RTOSPlusTCPEthernetInterface::TerminateDataPort()
{
    if(NumFtpResponders > 0)
    {
        sockets[FtpDataSocketNumber]->Terminate();
    }
}

void RTOSPlusTCPEthernetInterface::InitSockets()
{
	for (size_t i = 0; i < NumProtocols; ++i)
	{
		if (protocolEnabled[i])
		{
			StartProtocol(i);
		}
	}
	nextSocketToPoll = 0;
}

void RTOSPlusTCPEthernetInterface::TerminateSockets()
{
	for (SocketNumber skt = 0; skt < NumRTOSPlusTCPEthernetTcpSockets; ++skt)
	{
		sockets[skt]->Terminate();
	}
}



//*******  Handlers called from +TCP callbacks *******

//get the hostname
const char *RTOSPlusTCPEthernetInterface::ProcessApplicationHostnameHook( void ){
    return reprap.GetNetwork().GetHostname();//printerHostName;
}

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect events are only received if implemented in the MAC driver. */
void RTOSPlusTCPEthernetInterface::ProcessIPApplication( eIPCallbackEvent_t eNetworkEvent )
{
    //variables to hold IP information from +TCP layer (either from static assignment or DHCP)
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    
    FreeRTOS_debug_printf( ( "vApplicationIPNetworkEventHook: Event=%d \n", eNetworkEvent ) );
    
    
    /* If the network has just come up...*/
    if( eNetworkEvent == eNetworkUp )
    {
        
        
        /* Print out the network configuration, which may have come from a DHCP
         server. */
        FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );

//        char cBuffer[ 16 ];
//        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
//        FreeRTOS_debug_printf( ( "\r\n\r\nIP Address: %s\r\n", cBuffer ) );
//
//        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
//        FreeRTOS_debug_printf( ( "Subnet Mask: %s\r\n", cBuffer ) );
//
//        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
//        FreeRTOS_debug_printf( ( "Gateway Address: %s\r\n", cBuffer ) );
//
//        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
//        FreeRTOS_debug_printf( ( "DNS Server Address: %s\r\n\r\n\r\n", cBuffer ) );
        
        //If IP address still equals 0.0.0.0 here then DHCP has Failed.
        if( ulIPAddress == 0 ) //ulIPAddress is in 32bit format
        {
            state = NetworkState::obtainingIP;
            
            //TODO::
        }
        else
        {
            //update the class private vars with the values we got from +tcp
            //convert32bitIPAddress(ulIPAddress, ipAddress);
            //convert32bitIPAddress(ulNetMask, netmask);
            //convert32bitIPAddress(ulGatewayAddress, gateway);
            
            ipAddress.SetV4LittleEndian(ulIPAddress);
            netmask.SetV4LittleEndian(ulNetMask);
            gateway.SetV4LittleEndian(ulGatewayAddress);
            
            convert32bitIPAddress(ulDNSServerAddress, dnsServerAddress);

            state = NetworkState::connected; //set connected state (we have IP address)
        }

    }
    else if (eNetworkEvent == eNetworkDown)
    {
        
        //FreeRTOS_debug_printf( ( "Application Hook: NetDown\r\n" ) );
        state = NetworkState::establishingLink; //back to establishing link
        
    }

}


/* RTOS+TCP DHCP Hook.... this allows us to control the DHCP process.
   +TCP will automatically start a DHCP process (if DHCP is configured in config) and
   we need to use this hook to prevent DHCP starting if we have a static address set in config.g
*/
eDHCPCallbackAnswer_t RTOSPlusTCPEthernetInterface::ProcessDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress )
{
    FreeRTOS_debug_printf( ( "Process DHCP Hook") );

    eDHCPCallbackAnswer_t eReturn;
    
    /* This hook is called in a couple of places during the DHCP process, as identified by the eDHCPPhase parameter. */
    switch( eDHCPPhase )
    {
        case eDHCPPhasePreDiscover  :
            
            state = NetworkState::obtainingIP; // obtaining IP state
            
            
            /* A DHCP discovery is about to be sent out.  eDHCPContinue is returned to allow the discovery to go out.
             
             If eDHCPUseDefaults had been returned instead then the DHCP process would be stopped and the statically configured IP address would be used.
             
             If eDHCPStopNoChanges had been returned instead then the DHCP process would be stopped and
             whatever the current network configuration was would continue to be used.
             */
            
            
            if(ipAddress.GetV4LittleEndian() == 0)
            {
                eReturn = eDHCPContinue; //use DHCP
            }
            else
            {
                //RRF has been configured to use a static address, so dont attempt to use DHCP
                eReturn = eDHCPUseDefaults; //use Static
            }

            break;
            
        case eDHCPPhasePreRequest  :
            //            /* An offer has been received from the DHCP server, and the offered
            //             IP address is passed in the ulIPAddress parameter.  Convert the
            //             offered and statically allocated IP addresses to 32-bit values. */
            
            eReturn = eDHCPContinue; //continue with dhcp assigned address
            
            break;
            
        default :
            /* Cannot be reached, but set eReturn to prevent compiler warnings
             where compilers are disposed to generating one. */
            eReturn = eDHCPContinue;
            break;
    }
    
    return eReturn;

}


// End
