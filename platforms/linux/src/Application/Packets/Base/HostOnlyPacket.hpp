#ifndef BABLE_HOSTONLYPACKET_HPP
#define BABLE_HOSTONLYPACKET_HPP

#include "Application/Packets/AbstractPacket.hpp"

namespace Packet {

  class HostOnlyPacket : public AbstractPacket {

  public:
    static Packet::Type initial_type() {
      return Packet::Type::FLATBUFFERS;
    };

    static Packet::Type final_type() {
      return Packet::Type::FLATBUFFERS;
    };

  protected:
    HostOnlyPacket(Packet::Id id, uint16_t packet_code) : AbstractPacket(id, initial_type(), final_type(), packet_code) {
      m_native_class = "BaBLE";
    };

    void prepare(const std::shared_ptr<PacketRouter>& router) override {};

  };

}

#endif //BABLE_HOSTONLYPACKET_HPP
