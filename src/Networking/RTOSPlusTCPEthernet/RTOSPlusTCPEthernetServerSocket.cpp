/*
 *
 *
 *
 * Author: Stephen
 */

#include "RTOSPlusTCPEthernetServerSocket.h"
#include "RTOSPlusTCPEthernetInterface.h"
#include "Network.h"
#include "Networking/NetworkInterface.h"
#include "NetworkBuffer.h"
#include "RepRap.h"

RTOSPlusTCPEthernetServerSocket *RTOSPlusTCPEthernetServerSocket::serverSocketInstance = nullptr;


//***************************************************************************************************

/*static*/ RTOSPlusTCPEthernetServerSocket *RTOSPlusTCPEthernetServerSocket::Instance()
{
    
    if( serverSocketInstance == nullptr ){
        //setup instance
        serverSocketInstance = new RTOSPlusTCPEthernetServerSocket();
    }

    return serverSocketInstance;
    
}


RTOSPlusTCPEthernetServerSocket::RTOSPlusTCPEthernetServerSocket()
{
    for(NetworkProtocol i=0; i<NumProtocols; i++)
    {
        protocolServerSockets[i] = nullptr;
    }
}

// Initialise a TCP socket
Socket_t RTOSPlusTCPEthernetServerSocket::GetServerSocket(Port serverPort, NetworkProtocol p)
{
    static const TickType_t xReceiveTimeOut = 1; // time (in ticks) to wait for FreeRTOS_accept .. portMax_DELAY would wait indefinetly (block until client connection)
    static const TickType_t xSendTimeOut = 1;

    struct freertos_sockaddr xBindAddress;
    BaseType_t maxClients = 1; //max number of simultaneous connections allowed
    BaseType_t reuseSocket = pdFALSE; //

    
    if(p == HttpProtocol){
        maxClients = NumHttpSockets;
    }

    BaseType_t ret = FreeRTOS_issocketconnected(protocolServerSockets[p]);
    if( protocolServerSockets[p] != nullptr &&  ret >= 0 )
    {
        //we already have a non-reused Socket Setup for this protocol and its running
        return protocolServerSockets[p];
    }
    else
    {
        if(protocolServerSockets[p] != nullptr){
            FreeRTOS_debug_printf( ( "+TCPServerSocket::GetServerSocket(): Had non-resue socket but there is a problem: Error code: %d ", (int)ret) );
            CloseProtocol(p);
        }
        
    }
    
    FreeRTOS_debug_printf( ( "+TCPServerSocket::GetServerSocket(): port: %d, Protocol: %s, maxConnections: %d\n", serverPort, ProtocolNames[p], maxClients ) );
    
    //attempt to create the socket
    protocolServerSockets[p] = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
    
    if(protocolServerSockets[p] != FREERTOS_INVALID_SOCKET ){
        
        
        //TCP Settings
        
        /* If ipconfigUSE_TCP_WIN is set to 1 and FREERTOS_SO_WIN_PROPERTIES is to
         be used with FreeRTOS_setsockopt() to change the sliding window size from
         its default then do it here! (see the FreeRTOS_setsockopt() documentation. */
        
        //Sliding Window (must be enabled in Config)
        /* Declare an xWinProperties structure. */
        // xWinProperties_t  xWinProps;
        //    /* Clear the struct, just in case it will get extended in the future. */
        //    memset( &xWinProps, '\0', sizeof( xWinProps ) );
        //    /* Fill in the required buffer and window sizes. */
        //    xWinProps.lTxBufSize = 2 * ipconfigTCP_MSS; // Unit: bytes
        //    xWinProps.lTxWinSize = 2; // Unit: MSS
        //    xWinProps.lRxBufSize = 2 * ipconfigTCP_MSS; // Unit: bytes  two or more
        //    xWinProps.lRxWinSize = 2; // Unit: MSS
        //    FreeRTOS_setsockopt( xListeningSocket, 0, FREERTOS_SO_WIN_PROPERTIES, ( void * ) &xWinProps, sizeof( xWinProps ) );
        
        /* If FREERTOS_SO_RCVBUF or FREERTOS_SO_SNDBUF are to be used with FreeRTOS_setsockopt() to change the buffer sizes
         from their default then do it here!.  (see the FreeRTOS_setsockopt() documentation. */
        //This is not related to the size of the packets or the sliding window, it only sets the size of the buffer.
        //Buff size should be multiples of MSS
        
        uint32_t txbufferSize = 1*ipconfigTCP_MSS;
        uint32_t rxbufferSize = 1*ipconfigTCP_MSS;
        
        //set the Buffer size for RX and TX,
        FreeRTOS_setsockopt( protocolServerSockets[p], 0, FREERTOS_SO_RCVBUF, ( void * ) &rxbufferSize, sizeof( rxbufferSize ) );
        FreeRTOS_setsockopt( protocolServerSockets[p], 0, FREERTOS_SO_SNDBUF, ( void * ) &txbufferSize, sizeof( txbufferSize ) );
        
        
        //FREERTOS_SO_TCP_CONN_HANDLER - Install a callback for (dis) connection events. Supply pointer to 'F_TCP_UDP_Handler_t'
        //FREERTOS_SO_TCP_RECV_HANDLER - Install a callback for receiving TCP data. Supply pointer to 'F_TCP_UDP_Handler_t'
        //FREERTOS_SO_TCP_SENT_HANDLER - Install a callback for sending TCP data. Supply pointer to 'F_TCP_UDP_Handler_t'
        
        
        /* Set a time out so accept() will just wait for a connection. portMAX_DELAY will wait indefinetly */
        // set to a 1 tick... accept a connection if waiting or continue (non-blocking) - receive
        // set to 1 tick ... read from socket and wait 1 tick for data before continuing
        // This to be compatible with the existing Polled Networking
        FreeRTOS_setsockopt( protocolServerSockets[p], 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
        FreeRTOS_setsockopt( protocolServerSockets[p], 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );
        
        //Configure the socket reuse
        //If reuse is False (default) then the connecting sockect(s) will make a copy of the socket for simultaneous connected clients
        //else the same socket will be used at the accept socket and when closed, must be recreated again
        //
        FreeRTOS_setsockopt( protocolServerSockets[p], 0, FREERTOS_SO_REUSE_LISTEN_SOCKET, &reuseSocket, sizeof( reuseSocket ) );
        
        /* Set the listening port. */
        xBindAddress.sin_port = ( uint16_t ) serverPort;
        xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
        
        /* Bind the socket to the port */
        FreeRTOS_bind( protocolServerSockets[p], &xBindAddress, sizeof( xBindAddress ) );
        
        /* Set the socket into a listening state so it can accept connections.
         The maximum number of simultaneous connections is limited to value of backlog. */
        FreeRTOS_listen( protocolServerSockets[p], maxClients );
        
        return protocolServerSockets[p];
        
    }
    else
    {
        //failed to create socket
        debugPrintf( "Failed to create Protocol Server Socket for %s on port %d", ProtocolNames[p], serverPort );
        protocolServerSockets[p] = nullptr;

    }
    
    
    return nullptr;
}


void RTOSPlusTCPEthernetServerSocket::CloseProtocol(NetworkProtocol p)
{
    
    FreeRTOS_shutdown( protocolServerSockets[p], FREERTOS_SHUT_RDWR ); //perform graceful shutdown
    FreeRTOS_closesocket( protocolServerSockets[p] );
    protocolServerSockets[p] = nullptr;
}

void RTOSPlusTCPEthernetServerSocket::CloseAllProtocols()
{
    for(NetworkProtocol i=0; i<NumProtocols; i++ )
    {
        CloseProtocol(i);
    }
}

