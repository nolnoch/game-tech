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

class NetManager {
public:
  NetManager();
  virtual ~NetManager();

  bool initNetManager();
  bool openServer(int protocol, uint16 port);
  bool openClient(int protocol, char *addr, uint16 port);
  bool connectToServer();
  int  connectToClient();
  void dropServer();
  void dropClient();
  void close();

private:
  enum {
    NET_UNINITIALIZED = 0,
    NET_WAITING =       1,
    NET_RESOLVED =      2,
    NET_TCP_OPEN =      4,
    NET_UDP_OPEN =      8,
    NET_TCP_ACCEPT =    16,
    NET_UDP_BOUND =     32,
    NET_BLOCKED =       64,
    NET_SERVER =        256,
    NET_CLIENT =        512
  };
  static bool forceClientRandomUDP;
  int netStatus;
  int nUDPChannels, nClients;
  int boundChannel;
  uint16 tcpPorts[5], udpPorts[5];
  uint16 serverPort;
  IPaddress netServer;
  std::vector<IPaddress> netClients;
  std::vector<TCPsocket> tcpSockets;
  std::vector<UDPsocket> udpSockets;

  bool openTCPSocket (IPaddress *addr);
  bool openUDPSocket (uint16 port);
  bool acceptTCP(TCPsocket server);
  bool bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr);
  void unbindUDPSocket(UDPsocket sock, int channel);
  bool sendTCP(TCPsocket sock, const void *data, int len);
  bool sendUDP(UDPsocket sock, int channel, UDPpacket *pack);
  bool recvTCP(TCPsocket sock, void *data, int maxlen);
  bool recvUDP(UDPsocket sock, UDPpacket *pack);
  bool sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets);
  bool recvUDPV(UDPsocket sock, UDPpacket **packetV);
  void closeTCP(TCPsocket sock);
  void closeUDP(UDPsocket sock);
  IPaddress* queryTCPAddress(TCPsocket sock);
  IPaddress* queryUDPAddress(UDPsocket sock, int channel);

  // TODO make pointers to fields for auto-load?

  struct packet {
    // TODO add fields for game info.

  };
};

#endif /* NETMANAGER_H_ */
