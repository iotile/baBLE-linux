#ifndef BABLE_CENTRAL_WRITEWITHOUTRESPONSE_HPP
#define BABLE_CENTRAL_WRITEWITHOUTRESPONSE_HPP

#include "Application/Packets/Base/HostToControllerPacket.hpp"

namespace Packet {

  namespace Commands {

    namespace Central {

      class WriteWithoutResponse : public HostToControllerPacket {

      public:
        static const Packet::Type final_type() {
          return Packet::Type::HCI;
        };

        static const uint16_t initial_packet_code() {
          return static_cast<uint16_t>(BaBLE::Payload::WriteWithoutResponseCentral);
        };

        static const uint16_t final_packet_code() {
          return Format::HCI::AttributeCode::WriteCommand;
        };

        explicit WriteWithoutResponse(uint16_t attribute_handle = 0, std::vector<uint8_t> data = {});

        void unserialize(FlatbuffersFormatExtractor& extractor) override;
        std::vector<uint8_t> serialize(HCIFormatBuilder& builder) const override;
        std::vector<uint8_t> serialize(FlatbuffersFormatBuilder& builder) const override;

        void prepare(const std::shared_ptr<PacketRouter>& router) override;

        const std::string stringify() const override;

      private:
        uint16_t m_attribute_handle;
        std::vector<uint8_t> m_data_to_write;

        bool m_ack_to_send;
      };


    }

  }

}

#endif //BABLE_CENTRAL_WRITEWITHOUTRESPONSE_HPP
