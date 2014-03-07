/*
 * NetManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "NetManager.h"

NetManager::NetManager():
netStatus(0),
nUDPChannels(-1)
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
  if (netStatus ^ NET_RESOLVED)
    return false;

  if (forceClientRandomUDP && (netStatus & NET_CLIENT))
    port = PORT_RANDOM;

  UDPsocket udpSock = SDLNet_UDP_Open(port);

  if (!udpSock) {
    netStatus ^= NET_RESOLVED;
    netStatus |= NET_OPEN;
    udpSockets.push_back(udpSock);
  } else
    std::cout << "SDL_net: Failed to open UDP socket!" << std::endl;

  IPaddress *addr = (netStatus & NET_CLIENT) ? netServer : NULL;

  return bindUDPSocket(udpSock, nUDPChannels, addr);
}

bool NetManager::acceptTCP(TCPsocket server) {

}

bool NetManager::bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr) {
  bool ret = true;

  if (!addr) {
    // TODO Handle multiple clients.
  }

  boundChannel = SDLNet_UDP_Bind(sock, channel, addr);

  if (boundChannel == -1) {
    std::cout << "SDL_net: Failed to bind UDP socket to channel." << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::unbindUDPSocket(UDPsocket sock, int channel) {

}

bool NetManager::sendTCP(TCPsocket sock, const void *data, int len) {

}

bool NetManager::sendUDP(UDPsocket sock, int channel, UDPpacket *pack) {

}

bool NetManager::recvTCP(TCPsocket sock, void *data, int maxlen) {

}

bool NetManager::recvUDP(UDPsocket sock, UDPpacket *pack) {

}

bool NetManager::sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets) {

}

bool NetManager::recvUDPV(UDPsocket sock, UDPpacket **packetV) {

}

bool NetManager::closeTCP(TCPsocket sock) {

}

bool NetManager::closeUDP(UDPsocket sock) {

}

IPaddress* NetManager::queryTCPAddress(TCPsocket sock) {

}

IPaddress* NetManager::queryUDPAddress(UDPsocket sock, int channel) {

}


