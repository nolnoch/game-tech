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
netStatus(NET_UNINITIALIZED),
nextUDPChannel(CHANNEL_DEFAULT),
defaultPort(PORT_DEFAULT),
forceClientRandomUDP(true)
{
  if (!initNetManager())
    std::cout << "NetManager failed to initialize." << std::endl;
}

NetManager::~NetManager() {
  if (netStatus != NET_UNINITIALIZED)
    close();
  SDLNet_Quit();
}






/******************************************************************************
 * Private
 */


bool NetManager::initNetManager() {
  bool ret = true;

  if (SDL_Init(0)==-1) {
    printf("SDL_Init: %s\n", SDL_GetError());
    ret = false;
  } else if (SDLNet_Init()==-1) {
    printf("SDLNet_Init: %s\n", SDLNet_GetError());
    ret = false;
  } else {
    socketNursery = SDLNet_AllocSocketSet(SOCKET_ALL_MAX);

    if (!socketNursery) {
      std::cout << "SDL_net: Unable to allocate SocketSet." << std::endl;
      std::cout << SDLNet_GetError() << std::endl;
      ret = false;
    } else
      netStatus |= NET_WAITING;
  }

  return ret;
}

bool NetManager::startServer(int protocol, Uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if ((netStatus & NET_RESOLVED) && (netServer.protocols & protocol)) {
    std::cout << "NetManager: This protocol has already been established." << std::endl;
    return false;
  }

  if (netStatus & NET_CLIENT) {
    std::cout << "NetManager: Client already established. May not initiate server." << std::endl;
    return false;
  }

  int serverPort = port ? : defaultPort;

  if (SDLNet_ResolveHost(&netServer.address, NULL, serverPort)) {
    std::cout << "SDL_net: Failed to start server!" << std::endl;
  } else {
    netServer.protocols |= protocol;
    netStatus ^= NET_WAITING;
    netStatus |= (NET_SERVER | NET_RESOLVED);
  }

  return (protocol == PORT_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

bool NetManager::startClient(int protocol, char *hostname, Uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if ((netStatus & NET_RESOLVED) && (netServer.protocols & protocol)) {
    std::cout << "NetManager: This protocol has already been established." << std::endl;
    return false;
  }

  if (netStatus & NET_SERVER) {
    std::cout << "NetManager: Server already established. May not initiate client." << std::endl;
    return false;
  }

  int serverPort = port ? : defaultPort;

  if (SDLNet_ResolveHost(&netServer.address, hostname, serverPort)) {
    std::cout << "SDL_net: Failed to resolve server!" << std::endl;
  } else {
    netServer.protocols |= protocol;
    netServer.hostname = hostname;
    netStatus ^= NET_WAITING;
    netStatus |= NET_CLIENT | NET_RESOLVED;
  }

  return (protocol == PORT_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

bool NetManager::openTCPSocket(IPaddress *addr) {
  bool ret = false;

  if (statusCheck(NET_RESOLVED))
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Open(addr);

  if (!tcpSock)
    std::cout << "SDL_net: Failed to open TCP socket!" << std::endl;
  else {
    netServer.tcpSocketIdx = tcpSockets.size();
    netStatus |= NET_TCP_OPEN;
    tcpSockets.push_back(tcpSock);
    ret = true;
  }

  return (netStatus & NET_SERVER) ? acceptTCP(tcpSock) : ret;
}

bool NetManager::openUDPSocket(Uint16 port) {
  bool ret = true;
  Uint16 udpPort = port;

  if (statusCheck(NET_RESOLVED))
    return false;

  if ((netStatus & NET_CLIENT) && forceClientRandomUDP)
    udpPort = PORT_RANDOM;

  UDPsocket udpSock = SDLNet_UDP_Open(udpPort);

  if (!udpSock) {
    std::cout << "SDL_net: Failed to open UDP socket!" << std::endl;
    ret = false;
  } else {
    udpSockets.push_back(udpSock);

    if (statusCheck(NET_UDP_OPEN)) {
      netServer.udpSocketIdx = udpSockets.size() - 1;
      netStatus |= NET_UDP_OPEN;

      if (netStatus & NET_CLIENT)
        return bindUDPSocket(udpSock, nextUDPChannel++, &netServer.address);
    }
  }

  return ret;
}

bool NetManager::acceptTCP(TCPsocket server) {
  bool ret = false;

  if (statusCheck(NET_SERVER | NET_TCP_OPEN))
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Accept(server);

  if (tcpClients.size() >= SOCKET_TCP_MAX) {
    std::cout << "NetManager: Exceeded max number of TCP connections." << std::endl;
    rejectTCPClient(tcpSock);
    return false;
  }

  if (!tcpSock)
    std::cout << "SDL_net: Failed to accept TCP client on server socket." << std::endl;
  else {
    ConnectionInfo *client = new ConnectionInfo;
    MessageBuffer *buffer = new MessageBuffer;
    IPaddress *addr = queryTCPAddress(tcpSock);
    client->hostname = SDLNet_ResolveIP(addr);
    client->protocols |= PORT_TCP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->tcpSocketIdx = tcpSockets.size();
    client->clientIdx = tcpClients.size();
    buffer->host = addr->host;
    buffer->updated = false;
    tcpClientData.push_back(buffer);
    tcpClients.push_back(client);
    tcpSockets.push_back(tcpSock);

    netStatus |= NET_TCP_ACCEPT;
    ret = true;
  }

  return ret;
}

bool NetManager::bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr) {
  bool ret = true;

  if (statusCheck(NET_UDP_OPEN))
    return false;

  int udpchannel;
  udpchannel = SDLNet_UDP_Bind(sock, channel, addr);

  if (udpchannel == -1) {
    std::cout << "SDL_net: Failed to bind UDP address to channel on socket."
        << std::endl;
    ret = false;
  } else if (statusCheck(NET_UDP_BOUND)) {
    netServer.udpChannel = udpchannel;
    netStatus |= NET_UDP_BOUND;
  } else {
    ConnectionInfo *client = new ConnectionInfo;
    MessageBuffer *buffer = new MessageBuffer;
    client->hostname = SDLNet_ResolveIP(addr);
    client->protocols |= PORT_UDP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->udpSocketIdx = udpSockets.size() - 1;
    client->clientIdx = udpClients.size();
    client->udpChannel = udpchannel;
    buffer->host = addr->host;
    buffer->updated = false;
    udpClientData.push_back(buffer);
    udpClients.push_back(client);
  }

  return ret;
}

void NetManager::unbindUDPSocket(UDPsocket sock, int channel) {
  SDLNet_UDP_Unbind(sock, channel);
}

bool NetManager::sendTCP(TCPsocket sock, const void *data, int len) {
  bool ret = true;

  if (statusCheck((NET_TCP_ACCEPT), (NET_CLIENT | NET_TCP_OPEN)))
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

  if (statusCheck(NET_TCP_ACCEPT, (NET_CLIENT | NET_TCP_OPEN)))
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
  clearFlags(NET_TCP_ACCEPT | NET_TCP_OPEN);
  netServer.protocols ^= PORT_TCP;

  if (!netServer.protocols)
    close();
}

void NetManager::closeUDP(UDPsocket sock) {
  SDLNet_UDP_Close(sock);
  clearFlags(NET_UDP_BOUND | NET_UDP_OPEN);
  netServer.protocols ^= PORT_UDP;

  if (!netServer.protocols)
    close();
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

UDPpacket* NetManager::allocUDPpacket(int size) {
  UDPpacket *newPacket;

  newPacket = SDLNet_AllocPacket(size);

  if (!newPacket) {

  } else {

  }

  return newPacket;
}

int NetManager::resizeUDPpacket(UDPpacket *pack, int size) {

}

void NetManager::freeUDPpacket(UDPpacket *pack) {

}

void NetManager::watchSocket(TCPsocket *sock) {
  if (-1 == SDLNet_TCP_AddSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to add socket to SocketSet." << std::endl;
}

void NetManager::watchSocket(UDPsocket *sock) {
  if (-1 == SDLNet_UDP_AddSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to add socket to SocketSet." << std::endl;
}

void NetManager::unwatchSocket(TCPsocket *sock) {
  if (-1 == SDLNet_TCP_DelSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to remove socket from SocketSet." << std::endl;
}

void NetManager::unwatchSocket(UDPsocket *sock) {
  if (-1 == SDLNet_UDP_DelSocket(socketNursery, sock))
    std::cout << "SDL_net: Unable to remove socket from SocketSet." << std::endl;
}

void NetManager::checkSockets(Uint32 timeout_ms) {
  int nReadySockets;
  bool ret = false;

  nReadySockets = SDLNet_CheckSockets(socketNursery, timeout_ms);

  if (nReadySockets == -1) {
    std::cout << "SDL_net: System error in CheckSockets." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
  } else if (nReadySockets) {
    ret = true;
    int i = 0;

    if (netServer.protocols & PORT_TCP) {
      if (netStatus & NET_SERVER) {
        if (SDLNet_SocketReady(tcpSockets[netServer.tcpSocketIdx])) {
          acceptTCP(tcpSockets[netServer.tcpSocketIdx]);
          nReadySockets--;
        }
        for (i = 0; i < tcpClients.size() && nReadySockets; i++) {
          if (SDLNet_SocketReady(tcpSockets[tcpClients[i]->tcpSocketIdx])) {
            readTCPSocket(i);
            nReadySockets--;
          }
        }
      } else if (netStatus & NET_CLIENT) {
        if (SDLNet_SocketReady(tcpSockets[netServer.tcpSocketIdx])) {
          readTCPSocket(SOCKET_SELF);
          nReadySockets--;
        }
      }
    }
    if (netServer.protocols & PORT_UDP) {
      if (SDLNet_SocketReady(udpSockets[netServer.udpSocketIdx])) {
        readUDPSocket(SOCKET_SELF);
        nReadySockets--;
      }
      if (netStatus & NET_SERVER) {
        for (i = 0; i < udpClients.size() && nReadySockets; i++) {
          if (SDLNet_SocketReady(udpSockets[udpClients[i]->udpSocketIdx])) {
            readUDPSocket(i);
            nReadySockets--;
          }
        }
      }
    }
  }
}

void NetManager::readTCPSocket(int clientIdx) {
  bool result;
  int idxSocket, idxData;

  if (clientIdx == SOCKET_SELF) {
    idxSocket = netServer.tcpSocketIdx;
    idxData = netServer.clientIdx;
  } else {
    idxSocket = tcpClients[clientIdx]->tcpSocketIdx;
    idxData = clientIdx;
  }

  result = recvTCP(tcpSockets[idxSocket], tcpClientData[idxData]->buffer,
      MESSAGE_LENGTH);

  if (!result) {
    std::cout << "NetManager: Failed to read TCP packet from tcpClient " << idxData
        << "." << std::endl;
  } else
    tcpClientData[clientIdx]->updated = true;
}

void NetManager::readUDPSocket(int clientIdx) {
  UDPpacket *buf;
  bool result;
  int idxSocket, idxData;

  if (clientIdx == SOCKET_SELF) {
    idxSocket = netServer.udpSocketIdx;
    idxData = netServer.clientIdx;
  } else {
    idxSocket = udpClients[clientIdx]->udpSocketIdx;
    idxData = clientIdx;
  }

  buf = allocUDPpacket(MESSAGE_LENGTH);

  result = recvUDP(udpSockets[idxSocket], buf);

  if (!result) {
    std::cout << "NetManager: Failed to read UDP packet from udpClient "
        << clientIdx << "." << std::endl;
  } else {
    if (buf->channel == -1) {
      if (netStatus & NET_CLIENT)
        std::cout << "NetManager: Unrecognized packet source." << std::endl;
      else if (!addUDPClient(buf))
        std::cout << "NetManager: Unable to add new UDP client." << std::endl;
    } else {
      memcpy(udpClientData[idxData]->buffer, buf->data, buf->len);
      udpClientData[idxData]->updated = true;
    }
  }
  freeUDPpacket(buf);
}

bool NetManager::addUDPClient(UDPpacket *pack) {
  bool ret = true;
  int socketIdx;

  if (nextUDPChannel >= CHANNEL_MAX) {
    if (openUDPSocket(PORT_DEFAULT))
      nextUDPChannel = CHANNEL_DEFAULT;
    else {
      std::cout << "NetManager: Exceeded max number of UDP connections." << std::endl;
      rejectUDPClient(pack);
      return false;
    }
  } else
    bindUDPSocket(udpSockets.back(), nextUDPChannel++, &pack->address);

  return ret;
}

void NetManager::rejectTCPClient(TCPsocket sock) {
  // TODO Send rejection message in response packet.

  closeTCP(sock);
}

void NetManager::rejectUDPClient(UDPpacket *pack) {
  // TODO Send rejection message in response packet.

  freeUDPpacket(pack);
}

bool NetManager::statusCheck(int state) {
  bool ret = (state != (netStatus & state));

  if (ret)
    std::cout << "NetManager: Invalid state for command." << std::endl;

  return ret;
}

bool NetManager::statusCheck(int state1, int state2) {
  bool ret1 = (state1 != (netStatus & state1));
  bool ret2 = (state2 != (netStatus & state2));
  bool result = ret1 && ret2;

  if (result)
    std::cout << "NetManager: Invalid state for command." << std::endl;

  return result;
}

void NetManager::clearFlags(int state) {
  int mask = netStatus & state;
  netStatus ^= mask;
}



