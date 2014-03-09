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

static enum Protocol {
  PROTOCOL_TCP            = 1024,
  PROTOCOL_UDP            = 2048,
  PROTOCOL_ALL            = PROTOCOL_TCP | PROTOCOL_UDP
};

struct ConnectionInfo {
  int tcpSocketIdx;
  int udpSocketIdx;
  int clientIdx;
  int udpChannel;
  Protocol protocols;
  IPaddress address;
  std::string hostname;
};

struct MessageBuffer {
  Uint32 host;
  bool updated;
  char buffer[128];
};


class NetManager {
public:
  MessageBuffer serverData;
  std::vector<MessageBuffer *> tcpClientData;
  std::vector<MessageBuffer *> udpClientData;

  NetManager();
  virtual ~NetManager();

  bool initNetManager();
  void addNetworkInfo(Protocol protocol = PROTOCOL_ALL,
      Uint16 port = 0, const char *host = NULL);

  bool startServer();
  bool startClient();
  bool pollForActivity(Uint32 timeout_ms = 5000);
  bool scanForActivity();
  void messageClients(char *buf, int len);
  void messageServer(char *buf, int len);
  void messageClient(Protocol protocol, int clientDataIdx, char *buf, int len);
  void dropClient(Protocol protocol, int clientDataIdx);
  void stopServer(Protocol protocol);
  void stopClient(Protocol protocol);
  void close();

  bool addProtocol(Protocol protocol);
  void setProtocol(Protocol protocol);
  void setPort(Uint16 port);
  void setHost(const char *host);
  Uint32 getProtocol();
  Uint16 getPort();
  std::string getHost();


private:
  enum {
    NET_UNINITIALIZED   = 0,
    NET_INITIALIZED     = 1,
    NET_WAITING         = 2,
    NET_RESOLVED        = 4,
    NET_TCP_OPEN        = 8,
    NET_UDP_OPEN        = 16,
    NET_TCP_ACCEPT      = 32,
    NET_UDP_BOUND       = 64,

    NET_SERVER          = 256,
    NET_CLIENT          = 512,

    PORT_RANDOM         = 0,
    PORT_DEFAULT        = 51215,
    CHANNEL_AUTO        = -1,
    CHANNEL_DEFAULT     = 1,
    CHANNEL_MAX         = 2,    // Low for testing. Set to 6+ before launch.
    SOCKET_TCP_MAX      = 12,
    SOCKET_UDP_MAX      = 12,
    SOCKET_ALL_MAX      = SOCKET_TCP_MAX + SOCKET_UDP_MAX,
    SOCKET_SELF         = SOCKET_ALL_MAX + 1,
    MESSAGE_LENGTH      = 128
  };
  bool forceClientRandomUDP;
  bool acceptNewClients;
  int nextUDPChannel;
  int netStatus;
  int netPort, defaultPort;
  Protocol netProtocol;
  std::string netHost;
  ConnectionInfo netServer;
  std::vector<ConnectionInfo *> tcpClients;
  std::vector<ConnectionInfo *> udpClients;
  std::vector<TCPsocket> tcpSockets;
  std::vector<UDPsocket> udpSockets;
  SDLNet_SocketSet socketNursery;

  bool openServer(Protocol protocol, Uint16 port);
  bool openClient(Protocol protocol, std::string addr, Uint16 port);
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
  bool resizeUDPpacket(UDPpacket *pack, int size);
  void freeUDPpacket(UDPpacket **pack);

  void watchSocket(TCPsocket *sock);
  void watchSocket(UDPsocket *sock);
  void unwatchSocket(TCPsocket *sock);
  void unwatchSocket(UDPsocket *sock);
  void checkSockets(Uint32 timeout_ms);

  void readTCPSocket(int clientIdx);
  void readUDPSocket(int clientIdx);

  bool addUDPClient(UDPpacket *pack);
  void rejectTCPClient(TCPsocket sock);
  void rejectUDPClient(UDPpacket *pack);

  UDPpacket* craftUDPpacket(char *buf, int len);
  void processPacketData(const char *data);

  bool statusCheck(int state);
  bool statusCheck(int state1, int state2);
  void clearFlags(int state);
  void resetManager();

  // TODO make pointers to fields for auto-load of packets?

};

#endif /* NETMANAGER_H_ */
