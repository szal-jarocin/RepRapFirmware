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
#include "NetworkBufferManagement.h"

#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"
#include "RTOSIface.h"

constexpr size_t TcpPlusStackWords = ( uint16_t ) ipconfigIP_TASK_STACK_SIZE_WORDS+20; // needs to be aroud 240 when debugging with debugPrintf
static Task<TcpPlusStackWords> tcpPlusTask;

constexpr size_t EmacStackWords = 74+30;/*170*/ // needs to be bigger (>=170) if using debugPrinf for testing
static Task<EmacStackWords> emacTask;


RTOSPlusTCPEthernetInterface *rtosTCPEtherInterfacePtr; //pointer to the clas instance so we can call from the c hooks

extern "C"
{
    
    //SD:: use the RFF tasks management to create tasks needed for +tcp
    TaskHandle_t RRfInitialiseIPTask(TaskFunction_t pxTaskCode)
    {
        tcpPlusTask.Create(pxTaskCode, "IP-task", nullptr, ( UBaseType_t ) TaskBase::TcpPriority);
        return tcpPlusTask.GetHandle();
    }
    
    TaskHandle_t RRfInitialiseEMACTask(TaskFunction_t pxTaskCode)
    {
        emacTask.Create(pxTaskCode, "EMAC", nullptr, TaskBase::TcpPriority);
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


#if SUPPORT_OBJECT_MODEL

// Object model table and functions
// Note: if using GCC version 7.3.1 20180622 and lambda functions are used in this table, you must compile this file with option -std=gnu++17.
// Otherwise the table will be allocated in RAM instead of flash, which wastes too much RAM.

// Macro to build a standard lambda function that includes the necessary type conversions
#define OBJECT_MODEL_FUNC(_ret) OBJECT_MODEL_FUNC_BODY(RTOSPlusTCPEthernetInterface, _ret)

const ObjectModelTableEntry RTOSPlusTCPEthernetInterface::objectModelTable[] =
{
    // These entries must be in alphabetical order
    { "gateway", OBJECT_MODEL_FUNC(&(self->gateway)), TYPE_OF(IPAddress), ObjectModelTableEntry::none },
    { "ip", OBJECT_MODEL_FUNC(&(self->ipAddress)), TYPE_OF(IPAddress), ObjectModelTableEntry::none },
    { "name", OBJECT_MODEL_FUNC_NOSELF("RTOS+TCP"), TYPE_OF(const char *), ObjectModelTableEntry::none },
    { "netmask", OBJECT_MODEL_FUNC(&(self->netmask)), TYPE_OF(IPAddress), ObjectModelTableEntry::none },
};

DEFINE_GET_OBJECT_MODEL_TABLE(RTOSPlusTCPEthernetInterface)

#endif

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
#if ENABLE_FTP
		sockets[FtpSocketNumber]->Init(FtpSocketNumber, portNumbers[protocol], protocol);
#endif
		break;

	case TelnetProtocol:
#if ENABLE_TELNET
		sockets[TelnetSocketNumber]->Init(TelnetSocketNumber, portNumbers[protocol], protocol);
#endif
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
#if ENABLE_FTP
		sockets[FtpSocketNumber]->TerminateAndDisable();
		sockets[FtpDataSocketNumber]->TerminateAndDisable();
#endif
		break;

	case TelnetProtocol:
#if ENABLE_TELNET
		sockets[TelnetSocketNumber]->TerminateAndDisable();
#endif
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

    //set the global mac var needed for networkinterface.c
    ucMACAddress[0] = macAddress[0];
    ucMACAddress[1] = macAddress[1];
    ucMACAddress[2] = macAddress[2];
    ucMACAddress[3] = macAddress[3];
    ucMACAddress[4] = macAddress[4];
    ucMACAddress[5] = macAddress[5];

    uint8_t ip[4], nm[4], gw[4], dns[4];
    ipAddress.UnpackV4(ip);
    netmask.UnpackV4(nm);
    gateway.UnpackV4(gw);
    
    BaseType_t ret = FreeRTOS_IPInit( ip, nm, gw, dns, macAddress );
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


    
        //establishingLink and obtainingIP states are handled by the callback handlers from RTOS +TCP
        case NetworkState::establishingLink:
        case NetworkState::obtainingIP:
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
    switch (state)
    {
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
    
#if( ipconfigCHECK_IP_QUEUE_SPACE != 0 )
    platform.MessageF(mtype, "NetBuffers: %lu lowest: %lu\n", uxGetNumberOfFreeNetworkBuffers(), uxGetMinimumFreeNetworkBuffers() );
    //Print out the minimum IP Queue space left since boot
    platform.MessageF(mtype, "Lowest IP Event Queue: %lu of %d\n", uxGetMinimumIPQueueSpace(), ipconfigEVENT_QUEUE_LENGTH );
    
    //defined in driver for debugging
    extern uint32_t numNetworkRXIntOverrunErrors; //hardware producted overrun error
    extern uint32_t numNetworkDroppedPacketsDueToNoBuffer;
    extern uint8_t numNetworkUnalignedNetworkBuffers;
    
    platform.MessageF(mtype, "EthDrv: RX IntOverrun Errors: %lu\n", numNetworkRXIntOverrunErrors);
    platform.MessageF(mtype, "EthDrv: Dropped packets (no buffer): %lu\n",  numNetworkDroppedPacketsDueToNoBuffer );
    platform.MessageF(mtype, "EthDrv: Unaligned Network Buffers (should be 0): %d\n", numNetworkUnalignedNetworkBuffers);
    
    platform.MessageF(mtype, "EthDrv: EthFrameSize: %d (LPC Buffer Size: %d)\n", (uint16_t)(ipTOTAL_ETHERNET_FRAME_SIZE), (uint16_t)(ipTOTAL_ETHERNET_FRAME_SIZE+ipBUFFER_PADDING) );

# if defined(COLLECT_NETDRIVER_ERROR_STATS)
    
    extern uint32_t numNetworkCRCErrors;
    extern uint32_t numNetworkSYMErrors;
    extern uint32_t numNetworkLENErrors;
    extern uint32_t numNetworkALIGNErrors;
    extern uint32_t numNetworkOVERRUNErrors;
    
    platform.MessageF(mtype, "EthDrv: RX CRC Errors: %lu\n", numNetworkCRCErrors);
    platform.MessageF(mtype, "EthDrv: RX SYM Errors: %lu (PHY Reported an Error)\n", numNetworkSYMErrors);
    platform.MessageF(mtype, "EthDrv: RX LEN Errors: %lu (Frame length != actual data length)\n", numNetworkLENErrors);
    platform.MessageF(mtype, "EthDrv: RX ALIGN Errors: %lu\n", numNetworkALIGNErrors);
    platform.MessageF(mtype, "EthDrv: RX OVERRUN Errors: %lu\n", numNetworkOVERRUNErrors);
# endif

    
    
#endif
    
    
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

void RTOSPlusTCPEthernetInterface::SetIPAddress(IPAddress p_ip, IPAddress p_netmask, IPAddress p_gateway)
{
    ipAddress = p_ip;
    netmask = p_netmask;
    gateway = p_gateway;
}


void RTOSPlusTCPEthernetInterface::OpenDataPort(Port port)
{
#if ENABLE_FTP
	sockets[FtpDataSocketNumber]->Init(FtpDataSocketNumber, port, FtpDataProtocol);
#endif
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

        //If IP address still equals 0.0.0.0 here then DHCP has Failed.
        if( ulIPAddress == 0 ) //ulIPAddress is in 32bit format
        {
            state = NetworkState::obtainingIP;
            
            //TODO::
        }
        else
        {
            //update the class private vars with the values we got from +tcp
            ipAddress.SetV4LittleEndian(ulIPAddress);
            netmask.SetV4LittleEndian(ulNetMask);
            gateway.SetV4LittleEndian(ulGatewayAddress);
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
