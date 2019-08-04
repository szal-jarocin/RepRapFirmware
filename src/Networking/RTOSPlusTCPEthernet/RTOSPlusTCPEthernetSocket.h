/*
 * Socket.h
 *
 *  Created on: 25 Dec 2016
 *      Author: David
 */

#ifndef SRC_NETWORKING_RTOSPlusTCPEthernetSOCKET_H_
#define SRC_NETWORKING_RTOSPlusTCPEthernetSOCKET_H_

#include "RepRapFirmware.h"
#include "NetworkDefs.h"
#include "Socket.h"

#include <FreeRTOS.h>
#include "task.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "MessageType.h"

// Socket structure that we use to track TCP connections
class RTOSPlusTCPEthernetSocket : public Socket
{
public:
	RTOSPlusTCPEthernetSocket(NetworkInterface *iface);
	void Init(SocketNumber s, Port serverPort, NetworkProtocol p);

	void Poll(bool full) override;
	void Close() override;
	void Terminate() override;
	void TerminateAndDisable() override;
	bool ReadChar(char& c) override;
	bool ReadBuffer(const uint8_t *&buffer, size_t &len) override;
	void Taken(size_t len) override;
	bool CanRead() const override;
	bool CanSend() const override;
	size_t Send(const uint8_t *data, size_t length) override;
	void Send() override;
    void Diagnostics(MessageType mt) const;


private:
	void ReInit();
	void ReceiveData();
	void DiscardReceivedData();

	NetworkBuffer *receivedData;						// List of buffers holding received data
	uint32_t whenConnected;
	SocketNumber socketNum;								// The RTOSPlusTCPEthernet socket number we are using

//TCP+
    Socket_t xListeningSocket;  // server socket 
    Socket_t xConnectedSocket;  //connected client socket
    bool CheckSocketError(BaseType_t val); 

};

#endif /* SRC_NETWORKING_RTOSPlusTCPEthernetSOCKET_H_ */
