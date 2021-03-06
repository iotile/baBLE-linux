#ifndef BABLE_SOCKETCONTAINER_HPP
#define BABLE_SOCKETCONTAINER_HPP

#include <map>
#include "Transport/AbstractSocket.hpp"
#include "Application/Packets/AbstractPacket.hpp"

class SocketContainer {

public:
  // Register given socket in container
  SocketContainer& register_socket(std::shared_ptr<AbstractSocket> socket);

  std::shared_ptr<AbstractSocket> get_socket(Packet::Type packet_type, uint16_t controller_id = NON_CONTROLLER_ID) const;

  // Send a packet using the matching registered socket
  bool send(const std::shared_ptr<Packet::AbstractPacket>& packet);

private:
  std::map<std::tuple<Packet::Type, uint16_t>, std::shared_ptr<AbstractSocket>> m_sockets;

};

#endif //BABLE_SOCKETCONTAINER_HPP
