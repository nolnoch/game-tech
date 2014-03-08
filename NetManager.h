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


struct ConnectionInfo {
  int tcpSocketIdx;
  int udpSocketIdx;
  int clientIdx;
  int protocols;
  int udpChannel;
  IPaddress address;
  Uint16 port;
  std::string hostname;
};


class NetManager {
public:
  NetManager();
  virtual ~NetManager();

  bool startServerTCP();
  bool startServerUDP();
  bool listenForClient();
  void updateClientsTCP();
  void updateClientsUDP();
  void updateServerTCP();
  void updateServerUDP();
  void stopServer();
  void stopClient();
  void close();

  int getDefaultPort();
  int getDefaultUDPChannel();
  void setDefaultPort(int newPort);
  void setDefaultUDPChannel(int newChannel);


private:
  enum {
    NET_UNINITIALIZED   = 0,
    NET_WAITING         = 1,
    NET_RESOLVED        = 2,
    NET_TCP_OPEN        = 4,
    NET_UDP_OPEN        = 8,
    NET_TCP_ACCEPT      = 16,
    NET_UDP_BOUND       = 32,

    NET_SERVER          = 256,
    NET_CLIENT          = 512,
    PORT_TCP            = 1024,
    PORT_UDP            = 2048,

    PORT_RANDOM         = 0,
    PORT_DEFAULT        = 51215,
    CHANNEL_AUTO        = -1,
    CHANNEL_DEFAULT     = 1,
    MAX_SOCKETS         = 8
  };
  static bool forceClientRandomUDP;
  int netStatus;
  int defaultPort;
  int defaultUDPChannel;
  ConnectionInfo netServer;
  std::vector<ConnectionInfo *> tcpClients;
  std::vector<ConnectionInfo *> udpClients;
  std::vector<TCPsocket> tcpSockets;
  std::vector<UDPsocket> udpSockets;
  SDLNet_SocketSet socketNursery;

  bool initNetManager();
  bool startServer(int protocol, Uint16 port);
  bool startClient(int protocol, char *addr, Uint16 port);
  bool openTCPSocket (IPaddress *addr);
  bool openUDPSocket (Uint16 port);
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

  UDPpacket* allocUDPpacket(int size);
  int resizeUDPpacket(UDPpacket *pack, int size);
  void freeUDPpacket(UDPpacket *pack);

  void watchSocket(TCPsocket *sock);
  void watchSocket(UDPsocket *sock);
  void unwatchSocket(TCPsocket *sock);
  void unwatchSocket(UDPsocket *sock);
  void checkSockets(Uint32 timeout_ms);

  bool statusCheck(int state);
  bool statusCheck(int state1, int state2);
  void clearFlags(int state);

  // TODO make pointers to fields for auto-load?

};

#endif /* NETMANAGER_H_ */
