/*
 * NetManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: nolnoch
 */

#include "NetManager.h"



/******************************************************************************
 * Constructors/Destructors
 */


NetManager::NetManager():
netStatus(NET_UNINITIALIZED),
nextUDPChannel(CHANNEL_DEFAULT),
defaultPort(PORT_DEFAULT),
forceClientRandomUDP(true),
acceptNewClients(true),
socketNursery(0),
netProtocol(0),
netPort(0)
{
}

NetManager::~NetManager() {
  if (netStatus != NET_UNINITIALIZED)
    close();
  SDLNet_Quit();
}



/******************************************************************************
 * Public
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
      netStatus |= NET_INITIALIZED;
  }

  return ret;
}

void NetManager::addNetworkInfo(Protocol protocol, Uint16 port, const char *host) {
  if (statusCheck(NET_INITIALIZED)) {
    std::cout << "NetManager: Must initNetManager before proceeding." << std::endl;
    return;
  }

  setProtocol(protocol);
  setPort(port ? : defaultPort);
  if (host)
    setHost(host);

  netStatus |= NET_WAITING;
}

bool NetManager::startServer() {
  bool ret = true;

  if (statusCheck(NET_WAITING)) {
    std::cout << "NetManager: Must addNetworkInfo before starting server." << std::endl;
    return false;
  } else if (netStatus & NET_CLIENT) {
    std::cout << "NetManager: Client already started. May not start server." << std::endl;
    return false;
  } else if (!netHost.empty()) {
    std::cout << "NetManager: Host was specified. Are you sure you want to "
        "start a server?" << std::endl;
  }

  if (netProtocol & PROTOCOL_TCP) {
    ret = openServer(PROTOCOL_TCP, netPort);
  }
  if (netProtocol & PROTOCOL_UDP) {
    ret = ret || openServer(PROTOCOL_UDP, netPort);
  }

  return ret;
}

bool NetManager::startClient() {
  bool ret = false;

  if (statusCheck(NET_WAITING)) {
    std::cout << "NetManager: Must addNetworkInfo before starting client." << std::endl;
    return false;
  } else if (netStatus & NET_CLIENT) {
    std::cout << "NetManager: Server already started. May not start client." << std::endl;
    return false;
  } else if (netHost.empty()) {
    std::cout << "NetManager: Must specify a host to start a client." << std::endl;
    return false;
  }

  if (netProtocol & PROTOCOL_TCP) {
    ret = openClient(PROTOCOL_TCP, netHost, netPort);
  }
  if (netProtocol & PROTOCOL_UDP) {
    ret = ret || openClient(PROTOCOL_UDP, netHost, netPort);
  }

  return ret;
}

bool NetManager::pollForActivity(Uint32 timeout_ms) {
  if (statusCheck(NET_UDP_OPEN, NET_TCP_ACCEPT)) {
    std::cout << "NetManager: No established TCP or UDP sockets to poll." << std::endl;
    return false;
  }

  return checkSockets(timeout_ms);
}

bool NetManager::scanForActivity() {
  return pollForActivity(0);
}

void NetManager::messageClients(char *buf, int len) {
  int i;

  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: No server running, and thus no clients to message." << std::endl;
    return;
  }

  if (netServer.protocols & PROTOCOL_TCP) {
    for (i = 0; i < tcpClients.size(); i++) {
      sendTCP(tcpSockets[tcpClients[i]->tcpSocketIdx], buf, len);
    }
  }
  if (netServer.protocols & PROTOCOL_UDP) {
    UDPpacket *pack = craftUDPpacket(buf, len);

    if (pack)
      for (i = 0; i < udpClients.size(); i++) {
        sendUDP(udpSockets[udpClients[i]->udpSocketIdx],
            udpClients[i]->udpChannel, pack);
      }
  }
}

void NetManager::messageServer(char *buf, int len) {
  if (statusCheck(NET_CLIENT)) {
    std::cout << "NetManager: No client running, and thus no server to message." << std::endl;
    return;
  }

  if (netServer.protocols & PROTOCOL_TCP) {
    sendTCP(tcpSockets[netServer.tcpSocketIdx], buf, len);
  }
  if (netServer.protocols & PROTOCOL_UDP) {
    UDPpacket *pack = craftUDPpacket(buf, len);
    if (pack)
      sendUDP(udpSockets[netServer.udpSocketIdx], netServer.udpChannel, pack);
  }
}

void NetManager::messageClient(Protocol protocol, int clientDataIdx, char *buf, int len) {
  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: No server running, and thus no clients to message." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    TCPsocket client = tcpSockets[tcpClients[clientDataIdx]->tcpSocketIdx];
    sendTCP(client, buf, len);
  }
  if (protocol & PROTOCOL_UDP) {
    UDPpacket *pack = craftUDPpacket(buf, len);
    ConnectionInfo *cInfo = udpClients[clientDataIdx];
    UDPsocket client = udpSockets[cInfo->udpSocketIdx];
    if (pack)
      sendUDP(client, cInfo->udpChannel, pack);
  }
}

void NetManager::dropClient(Protocol protocol, int clientDataIdx) {
  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: No server running, and thus no clients to drop." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    TCPsocket client = tcpSockets[tcpClients[clientDataIdx]->tcpSocketIdx];
    unwatchSocket(&client);
    closeTCP(client);
    tcpSockets.erase(tcpSockets.begin() + clientDataIdx);
  }
  if (protocol & PROTOCOL_UDP) {
    ConnectionInfo *cInfo = udpClients[clientDataIdx];
    UDPsocket client = udpSockets[cInfo->udpSocketIdx];
    unbindUDPSocket(client, cInfo->udpChannel);

    // TODO Implement reclaimable channels through bitmap or 2d array?
  }
}

void NetManager::stopServer(Protocol protocol) {
  int i;

  if (statusCheck(NET_SERVER)) {
    std::cout << "NetManager: There's no server running, dummy." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    for (i = 0; i < tcpClients.size(); i++) {
      TCPsocket client = tcpSockets[i];
      unwatchSocket(&client);
      closeTCP(client);
      tcpSockets.erase(tcpSockets.begin() + i);
    }
  }
  if (protocol & PROTOCOL_UDP) {
    for (i = 0; i < udpClients.size(); i++) {
      ConnectionInfo *cInfo = udpClients[i];
      UDPsocket client = udpSockets[cInfo->udpSocketIdx];
      unbindUDPSocket(client, cInfo->udpChannel);
    }
    for (i = udpSockets.size() - 1; i > 0; i--) {
      unwatchSocket(&udpSockets[i]);
      closeUDP(udpSockets[i]);
      udpSockets.pop_back();
    }
  }
  SDLNet_FreeSocketSet(socketNursery);

  resetManager();
}

void NetManager::stopClient(Protocol protocol) {
  if (statusCheck(NET_CLIENT)) {
    std::cout << "NetManager: You're not a client, thus you can't be stopped." << std::endl;
    return;
  }

  if (protocol & PROTOCOL_TCP) {
    TCPsocket server = tcpSockets[netServer.tcpSocketIdx];
    unwatchSocket(&server);
    closeTCP(server);
    tcpSockets.pop_back();
  }
  if (protocol & PROTOCOL_UDP) {
    UDPsocket server = udpSockets[netServer.udpSocketIdx];
    unwatchSocket(&server);
    unbindUDPSocket(server, netServer.udpChannel);
    closeUDP(server);
    udpSockets.pop_back();
  }
  SDLNet_FreeSocketSet(socketNursery);

  resetManager();
}

void NetManager::close() {
  if (netStatus & NET_SERVER) {
    stopServer(netServer.protocols);
  } else if (netStatus & NET_CLIENT) {
    stopClient(netServer.protocols);
  } else {
    std::cout << "NetManager: No active server or client. Nothing to do." << std::endl;
  }
}

bool NetManager::addProtocol(Protocol protocol) {
  netProtocol |= protocol;
  netStatus |= NET_WAITING;

  return (netStatus & NET_SERVER) ? startServer() : startClient();
}

void NetManager::setProtocol(Protocol protocol) {
  netProtocol = protocol;
}

void NetManager::setPort(Uint16 port) {
  netPort = port;
}

void NetManager::setHost(const char *host) {
  netHost = std::string(host);
}

Uint32 NetManager::getProtocol() {
  return (Uint32) netProtocol;
}

Uint16 NetManager::getPort() {
  return netPort;
}

std::string NetManager::getHost() {
  return netHost;
}




/******************************************************************************
 * Private
 */


bool NetManager::openServer(Protocol protocol, Uint16 port) {
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

  if (SDLNet_ResolveHost(&netServer.address, NULL, port)) {
    std::cout << "SDL_net: Failed to start server!" << std::endl;
  } else {
    netServer.protocols |= protocol;
    netStatus ^= NET_WAITING;
    netStatus |= NET_RESOLVED;
  }

  return (protocol & PROTOCOL_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

bool NetManager::openClient(Protocol protocol, std::string hostname, Uint16 port) {
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

  if (SDLNet_ResolveHost(&netServer.address, hostname.c_str(), port)) {
    std::cout << "SDL_net: Failed to resolve server!" << std::endl;
  } else {
    netServer.protocols |= protocol;
    netServer.hostname = hostname;
    netStatus ^= NET_WAITING;
    netStatus |= NET_RESOLVED;
  }

  return (protocol & PROTOCOL_TCP) ?
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
    watchSocket(&tcpSock);
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
    watchSocket(&udpSock);

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

  if (!acceptNewClients || (tcpClients.size() >= SOCKET_TCP_MAX)) {
    if (!acceptNewClients)
      std::cout << "NetManager: TCP client rejected. Not accepting new clients." << std::endl;
    else
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
    client->protocols |= PROTOCOL_TCP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->tcpSocketIdx = tcpSockets.size();
    client->clientIdx = tcpClients.size();
    buffer->host = addr->host;
    buffer->updated = false;
    tcpClientData.push_back(buffer);
    tcpClients.push_back(client);
    tcpSockets.push_back(tcpSock);
    watchSocket(&tcpSock);

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
    client->protocols |= PROTOCOL_UDP;
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

  if (statusCheck(NET_TCP_ACCEPT, (NET_CLIENT | NET_TCP_OPEN)))
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

  if (statusCheck(NET_UDP_BOUND) || !pack)
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
  netServer.protocols ^= PROTOCOL_TCP;

  if (!netServer.protocols)
    close();
}

void NetManager::closeUDP(UDPsocket sock) {
  SDLNet_UDP_Close(sock);
  clearFlags(NET_UDP_BOUND | NET_UDP_OPEN);
  netServer.protocols ^= PROTOCOL_UDP;

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
    std::cout << "SDL_net: Unable to allocate UDP packet." << std::endl;
  }

  return newPacket;
}

bool NetManager::resizeUDPpacket(UDPpacket *pack, int size) {
  bool ret = true;
  int newSize;

  newSize = SDLNet_ResizePacket(pack, size);

  if (newSize < size) {
    std::cout << "SDL_net: Unable to resize UDP packet as requested." << std::endl;
    std::cout << SDLNet_GetError() << std::endl;
    ret = false;
  }

  return ret;
}

void NetManager::freeUDPpacket(UDPpacket **pack) {
  SDLNet_FreePacket(*pack);
  *pack = NULL;
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

    if (netServer.protocols & PROTOCOL_TCP) {
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
    if (netServer.protocols & PROTOCOL_UDP) {
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
  int idxSocket;
  MessageBuffer *mBuf;

  if (clientIdx == SOCKET_SELF) {
    idxSocket = netServer.tcpSocketIdx;
    mBuf = (netStatus & NET_SERVER) ? tcpClientData[netServer.clientIdx] : &serverData;
  } else {
    idxSocket = tcpClients[clientIdx]->tcpSocketIdx;
    mBuf = tcpClientData[clientIdx];
  }

  result = recvTCP(tcpSockets[idxSocket], mBuf->buffer,
      MESSAGE_LENGTH);

  if (!result) {
    std::cout << "NetManager: Failed to read TCP packet from tcpClient " << clientIdx
        << "." << std::endl;
  } else
    mBuf->updated = true;
}

void NetManager::readUDPSocket(int clientIdx) {
  UDPpacket *buf;
  bool result;
  int idxSocket;
  MessageBuffer *mBuf;

  if (clientIdx == SOCKET_SELF) {
    idxSocket = netServer.udpSocketIdx;
    mBuf = (netStatus & NET_SERVER) ? udpClientData[netServer.clientIdx] : &serverData;
  } else {
    idxSocket = udpClients[clientIdx]->udpSocketIdx;
    mBuf = udpClientData[clientIdx];
  }

  buf = allocUDPpacket(MESSAGE_LENGTH);

  result = recvUDP(udpSockets[idxSocket], buf);

  if (!result) {
    std::cout << "NetManager: Failed to read UDP packet from udpClient "
        << clientIdx << "." << std::endl;
  } else {
    if (buf->channel == -1) {
      if (netStatus & NET_CLIENT) {
        std::cout << "NetManager: Unrecognized packet source." << std::endl;
      } else if (!addUDPClient(buf)) {
        std::cout << "NetManager: Unable to add new UDP client." << std::endl;
      }
    } else {
      memcpy(mBuf->buffer, buf->data, buf->len);
      mBuf->updated = true;
    }
  }

  if (buf)
    freeUDPpacket(&buf);
}

bool NetManager::addUDPClient(UDPpacket *pack) {
  bool ret = true;
  int socketIdx;

  if (!acceptNewClients) {
    std::cout << "NetManager: UDP client rejected. Not accepting new clients." << std::endl;
    rejectUDPClient(pack);
    return false;
  }

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

  freeUDPpacket(&pack);
}

UDPpacket* NetManager::craftUDPpacket(char *buf, int len) {
  UDPpacket *packet;
  int header;

  if (len > MESSAGE_LENGTH) {
    std::cout << "NetManager: Message length exceeds current maximum." << std::endl;
    return NULL;
  }

  packet = allocUDPpacket(MESSAGE_LENGTH);

  if (!packet)
    return NULL;

  packet->len = len;
  memcpy(packet->data, buf, len);

  return packet;
}

void NetManager::processPacketData(const char *data) {
  // TODO Scan copied data to check for messages to NetManager.

  // TODO Establish clear signals for 'drop client' et al.
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

void NetManager::resetManager() {
  int i;

  for (i = tcpClientData.size(); i >= 0; i--) {
    delete tcpClientData[i];
    tcpClientData.pop_back();
  }
  for (i = udpClientData.size(); i >= 0; i--) {
    delete udpClientData[i];
    udpClientData.pop_back();
  }
  for (i = tcpClients.size(); i >= 0; i--) {
    delete tcpClients[i];
    tcpClients.pop_back();
  }
  for (i = udpClients.size(); i >= 0; i--) {
    delete udpClients[i];
    udpClients.pop_back();
  }

  forceClientRandomUDP = true;
  acceptNewClients = true;
  nextUDPChannel = 1;
  netStatus = NET_UNINITIALIZED;
  netPort = PORT_DEFAULT;
  netProtocol = PROTOCOL_ALL;
  netHost.clear();
}



