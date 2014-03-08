/*
 * NetManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "NetManager.h"


/******************************************************************************
 * Public
 */


NetManager::NetManager():
netStatus(0),
nClients(0),
nUDPChannels(-1),
forceClientRandomUDP(true)
{
  initNetManager();
}

NetManager::~NetManager() {
  SDLNet_Quit();
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

bool NetManager::startServer(int protocol, uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if (SDLNet_ResolveHost(&netServer.address, NULL, port)) {
    std::cout << "SDL_net: Failed to start server!" << std::endl;
  } else {
    netServer.protocol = protocol;
    netServer.port = port;
    netStatus = NET_SERVER | NET_RESOLVED;
  }

  return (protocol == PORT_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

bool NetManager::startClient(int protocol, char *hostname, uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if (SDLNet_ResolveHost(&netServer.address, hostname, port)) {
    std::cout << "SDL_net: Failed to resolve server!" << std::endl;
  } else {
    netServer.protocol = protocol;
    netServer.hostname = hostname;
    netServer.port = port;
    netStatus = NET_CLIENT | NET_RESOLVED;
  }

  return (protocol == PORT_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

int  NetManager::addClient() {

}

void NetManager::dropServer() {

}

void NetManager::dropClient() {

}

void NetManager::close() {

}



/******************************************************************************
 * Private
 */


bool NetManager::openTCPSocket(IPaddress *addr) {
  bool ret = false;

  if (statusCheck(NET_RESOLVED))
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Open(addr);

  if (!tcpSock)
    std::cout << "SDL_net: Failed to open TCP socket!" << std::endl;
  else {
    netServer.socketList = &tcpSockets;
    netServer.socketIdx = tcpSockets.size();
    netStatus ^= NET_RESOLVED;
    netStatus |= NET_TCP_OPEN;
    tcpSockets.push_back(tcpSock);
    ret = true;
  }

  return (netStatus & NET_SERVER) ? acceptTCP(tcpSock) : ret;
}

bool NetManager::openUDPSocket(uint16 port) {
  uint16 udpPort = port;

  if (statusCheck(NET_RESOLVED))
    return false;

  if ((netStatus & NET_CLIENT) && forceClientRandomUDP)
    udpPort = PORT_RANDOM;

  UDPsocket udpSock = SDLNet_UDP_Open(udpPort);

  if (!udpSock)
    std::cout << "SDL_net: Failed to open UDP socket!" << std::endl;
  else {
    netServer.socketList = &udpSockets;
    netServer.socketIdx = udpSockets.size();
    udpSockets.push_back(udpSock);
    netStatus ^= NET_RESOLVED;
    netStatus |= NET_UDP_OPEN;
  }

  IPaddress *addr = (netStatus & NET_CLIENT) ? netServer.address : NULL;

  return bindUDPSocket(udpSock, nUDPChannels, addr);
}

bool NetManager::acceptTCP(TCPsocket server) {
  bool ret = false;

  if (statusCheck(NET_SERVER | NET_TCP_OPEN))
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Accept(server);

  if (!tcpSock)
    std::cout << "SDL_net: Failed to accept TCP client on server socket." << std::endl;
  else {
    ConnectionInfo *client = new ConnectionInfo;
    IPaddress *addr = queryTCPAddress(tcpSock);
    client->hostname = SDLNet_ResolveIP(addr);
    client->protocol = PORT_TCP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->socketList = &tcpSockets;
    client->socketIdx = tcpSockets.size();
    client->clientIdx = netClients.size();
    netClients.push_back(client);
    tcpSockets.push_back(tcpSock);

    netStatus ^= NET_TCP_OPEN;
    netStatus |= NET_TCP_ACCEPT;
    ret = true;
  }

  return ret;
}

bool NetManager::bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr) {
  bool ret = true;

  if (statusCheck(NET_UDP_OPEN))
    return false;

  if (netStatus & NET_SERVER) {
    int i;

    if (netClients.empty()) {
      std::cout << "NetManager: No clients to bind for UDP!" << std::endl;
      ret = false;
    }

    for (i = 0; i < netClients.size() && ret; i ++) {
      if (-1 == SDLNet_UDP_Bind(sock, i, &netClients[i]->address)) {
        std::cout << "SDL_net: Failed to bind UDP socket to channel." << std::endl;
        ret = false;
      }
    }
  } else if (netStatus & NET_CLIENT) {
    serverUDPChannel = SDLNet_UDP_Bind(sock, channel, addr);

    if (serverUDPChannel == -1) {
      std::cout << "SDL_net: Failed to bind UDP socket to channel." << std::endl;
      ret = false;
    }
  } else {
    std::cout << "NetManager: Not a client or server. This cannot happen."
        << std::endl;
    ret = false;
  }

  return ret;
}

void NetManager::unbindUDPSocket(UDPsocket sock, int channel) {
  SDLNet_UDP_Unbind(sock, channel);
}

bool NetManager::sendTCP(TCPsocket sock, const void *data, int len) {
  bool ret = true;

  if (statusCheck(NET_CLIENT | NET_TCP_ACCEPT) ||
      statusCheck(NET_SERVER | NET_TCP_OPEN))
    return false;

  if (len > SDLNet_TCP_Send(sock, data, len)) {
    std::cout << "SDL_net: Failed to send TCP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::sendUDP(UDPsocket sock, int channel, UDPpacket *pack) {
  bool ret = true;

  if (statusCheck(NET_UDP_BOUND))
    return false;

  if (!SDLNet_UDP_Send(sock, channel, pack)) {
    std::cout << "SDL_net: Failed to send UDP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::recvTCP(TCPsocket sock, void *data, int maxlen) {
  bool ret = true;

  if (statusCheck(NET_CLIENT | NET_TCP_ACCEPT) ||
      statusCheck(NET_SERVER | NET_TCP_OPEN))
    return false;

  if (0 >= SDLNet_TCP_Recv(sock, data, maxlen)) {
    std::cout << "SDL_net: Failed to receive TCP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::recvUDP(UDPsocket sock, UDPpacket *pack) {
  bool ret = true;
  int result;

  if (statusCheck(NET_UDP_BOUND))
    return false;

  result  = SDLNet_UDP_Recv(sock, pack);

  if (result < 1) {
    ret = false;

    if (result < 0) {
      std::cout << "SDL_net: Failed to receive UDP data." << std::endl;
      std::cout << SDLNet_GetError() << std::endl;
    } else
      std::cout << "NetManager: No packets received." << std::endl;
  }

  return ret;
}

bool NetManager::sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets) {
  bool ret = true;

  if (statusCheck(NET_UDP_BOUND))
    return false;

  if (!SDLNet_UDP_SendV(sock, packetV, npackets)) {
    std::cout << "SDL_net: Failed to send UDP data." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

bool NetManager::recvUDPV(UDPsocket sock, UDPpacket **packetV) {
  bool ret = true;
  int result;

  if (statusCheck(NET_UDP_BOUND))
    return false;

  result  = SDLNet_UDP_RecvV(sock, packetV);

  if (result < 1) {
    ret = false;

    if (result < 0) {
      std::cout << "SDL_net: Failed to receive UDP data." << std::endl;
      std::cout << SDLNet_GetError() << std::endl;
    } else
      std::cout << "NetManager: No packets received." << std::endl;
  }

  return ret;
}

void NetManager::closeTCP(TCPsocket sock) {
  SDLNet_TCP_Close(sock);
}

void NetManager::closeUDP(UDPsocket sock) {
  SDLNet_UDP_Close(sock);
}

IPaddress* NetManager::queryTCPAddress(TCPsocket sock) {
  IPaddress *remote;

  remote = SDLNet_TCP_GetPeerAddress(sock);

  if (!remote) {
    std::cout << "SDL_net: Error retrieving remote TCP IP/port."
        "  This may be a server socket." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
  }

  return remote;
}

IPaddress* NetManager::queryUDPAddress(UDPsocket sock, int channel) {
  IPaddress *remote;

  remote = SDLNet_UDP_GetPeerAddress(sock, channel);

  if (!remote) {
    std::cout << "SDL_net: Error retrieving remote UDP IP address." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
  }

  return remote;
}

bool NetManager::statusCheck(int state) {
  bool ret = (state != (netStatus & state));

  if (ret) {
    std::cout << "NetManager: Invalid state for command." << std::endl;
  }

  return ret;
}

