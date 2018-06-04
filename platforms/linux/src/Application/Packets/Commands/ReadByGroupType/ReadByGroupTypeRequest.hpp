#ifndef BABLE_LINUX_READBYGROUPTYPEREQUEST_HPP
#define BABLE_LINUX_READBYGROUPTYPEREQUEST_HPP

#include "../RequestPacket.hpp"

namespace Packet {

  namespace Commands {

    class ReadByGroupTypeRequest : public RequestPacket<ReadByGroupTypeRequest> {

    public:
      static const uint16_t packet_code(Packet::Type type) {
        switch (type) {
          case Packet::Type::MGMT:
            throw std::invalid_argument("'ReadByGroupTypeRequest' packet is not compatible with MGMT protocol.");

          case Packet::Type::HCI:
            return Format::HCI::AttributeCode::ReadByGroupTypeRequest;

          case Packet::Type::FLATBUFFERS:
            throw std::invalid_argument("'ReadByGroupTypeRequest' packet is not compatible with Flatbuffers protocol.");

          case Packet::Type::NONE:
            return 0;
        }
      };

      ReadByGroupTypeRequest(Packet::Type initial_type, Packet::Type translated_type);

      std::vector<uint8_t> serialize(HCIFormatBuilder& builder) const override;

      const std::string stringify() const override;

      void before_sent(const std::shared_ptr<PacketRouter>& router) override;
      std::shared_ptr<Packet::AbstractPacket> on_response_received(const std::shared_ptr<PacketRouter>& router,
                                                                   const std::shared_ptr<AbstractPacket>& packet) override;
      std::shared_ptr<Packet::AbstractPacket> on_error_response_received(const std::shared_ptr<PacketRouter>& router,
                                                                         const std::shared_ptr<AbstractPacket>& packet);

      void set_handles(uint16_t starting_handle, uint16_t ending_handle);

    private:
      uint16_t m_starting_handle;
      uint16_t m_ending_handle;
      uint16_t m_uuid;
    };

  }

}


#endif //BABLE_LINUX_READBYGROUPTYPEREQUEST_HPP
