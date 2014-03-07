/*
 * NetManager.h
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#ifndef NETMANAGER_H_
#define NETMANAGER_H_


#include <vector>
#include <SDL/SDL_net.h>


typedef unsigned short int uint16;
enum {PORT_RANDOM = 0, PORT_TCP = 1020, PORT_UDP = 1040};

const bool forceClientRandomUDP = true;

class NetManager {
public:
  NetManager();
  virtual ~NetManager();

  bool initNetManager();
  bool openServer(int protocol, uint16 port);
  bool openClient(int protocol, char *addr, uint16 port);
  bool openTCPSocket (IPaddress *addr);
  bool openUDPSocket (uint16 port);
  bool acceptTCP(TCPsocket server);
  bool bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr);
  bool unbindUDPSocket(UDPsocket sock, int channel);
  bool sendTCP(TCPsocket sock, const void *data, int len);
  bool sendUDP(UDPsocket sock, int channel, UDPpacket *pack);
  bool recvTCP(TCPsocket sock, void *data, int maxlen);
  bool recvUDP(UDPsocket sock, UDPpacket *pack);
  bool sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets);
  bool recvUDPV(UDPsocket sock, UDPpacket **packetV);
  bool closeTCP(TCPsocket sock);
  bool closeUDP(UDPsocket sock);
  IPaddress* queryTCPAddress(TCPsocket sock);
  IPaddress* queryUDPAddress(UDPsocket sock, int channel);

private:
  enum {NET_UNINITIALIZED = 0, NET_WAITING = 1, NET_RESOLVED = 2, NET_OPEN = 4,
    NET_BOUND = 8, NET_BLOCKED = 16, NET_SERVER = 256, NET_CLIENT = 512};
  int tcpPorts[5], udpPorts[5];
  int netStatus, nUDPChannels;
  int boundChannel;
  uint16 serverPort;
  IPaddress netServer;
  std::vector<IPaddress> netClients;
  std::vector<TCPsocket> tcpSockets;
  std::vector<UDPsocket> udpSockets;

  // TODO make pointers to fields for auto-load?

  struct packet {
    // TODO add fields for game info.

  };
};

#endif /* NETMANAGER_H_ */
