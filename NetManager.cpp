/*
 * NetManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "NetManager.h"

NetManager::NetManager():
netStatus(0)
{
  initNetManager();
}

NetManager::~NetManager() {
  SDL_Quit();
}

bool NetManager::initNetManager() {
  bool ret = true;

  if (SDL_Init(0)==-1) {
    printf("SDL_Init: %s\n", SDL_GetError());
    ret = false;
  } else if (SDLNet_Init()==-1) {
    printf("SDLNet_Init: %s\n", SDLNet_GetError());
    ret = false;
  } else
    netStatus |= NET_WAITING;

  return ret;
}

bool NetManager::openServer(int protocol, uint16 port) {
  if (netStatus ^ NET_WAITING)
    return false;

  if (!SDLNet_ResolveHost(&netServer, NULL, serverPort))
    netStatus = NET_SERVER | NET_RESOLVED;
  else
    std::cout << "SDL_net: Failed to open server!" << std::endl;

  return (protocol == PORT_TCP) ?
      openTCPSocket(&netServer) : openUDPSocket(port);
}

bool NetManager::openClient(int protocol, char *addr, uint16 port) {
  if (netStatus ^ NET_WAITING)
    return false;

  if (!SDLNet_ResolveHost(&netServer, addr, port))
    netStatus = NET_CLIENT | NET_RESOLVED;
  else
    std::cout << "SDL_net: Failed to connect to server!" << std::endl;

  return (protocol == PORT_TCP) ?
      openTCPSocket(&netServer) : openUDPSocket(port);
}

bool NetManager::openTCPSocket(IPaddress *addr) {
  bool ret = false;

  if (netStatus ^ NET_RESOLVED)
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Open(addr);

  if ((ret = !tcpSock)) {
    netStatus ^= NET_RESOLVED;
    netStatus |= NET_OPEN;
    tcpSockets.push_back(tcpSock);
  } else
    std::cout << "SDL_net: Failed to open TCP socket!" << std::endl;

  return ret;
}

bool NetManager::openUDPSocket(uint16 port) {
  bool ret = false;

  if (netStatus ^ NET_RESOLVED)
    return false;

  if (forceClientRandomUDP && (netStatus & NET_CLIENT))
    port = PORT_RANDOM;

  UDPsocket udpSock = SDLNet_UDP_Open(port);

  if ((ret = !udpSock)) {
    netStatus ^= NET_RESOLVED;
    netStatus |= NET_OPEN;
    udpSockets.push_back(udpSock);
  } else
    std::cout << "SDL_net: Failed to open UDP socket!" << std::endl;

  return ret;
}

bool acceptTCP(TCPsocket server) {

}

bool bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr) {

}

bool unbindUDPSocket(UDPsocket sock, int channel) {

}

bool sendTCP(TCPsocket sock, const void *data, int len) {

}

bool sendUDP(UDPsocket sock, int channel, UDPpacket *pack) {

}

bool recvTCP(TCPsocket sock, void *data, int maxlen) {

}

bool recvUDP(UDPsocket sock, UDPpacket *pack) {

}

bool sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets) {

}

bool recvUDPV(UDPsocket sock, UDPpacket **packetV) {

}

bool closeTCP(TCPsocket sock) {

}

bool closeUDP(UDPsocket sock) {

}

IPaddress* queryTCPAddress(TCPsocket sock) {

}

IPaddress* queryUDPAddress(UDPsocket sock, int channel) {

}


