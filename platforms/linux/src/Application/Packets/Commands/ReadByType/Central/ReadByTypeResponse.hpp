#ifndef BABLE_CENTRAL_READBYTYPERESPONSE_HPP
#define BABLE_CENTRAL_READBYTYPERESPONSE_HPP

#include "Application/Packets/Base/ControllerToHostPacket.hpp"

namespace Packet {

  namespace Commands {

    namespace Central {

      class ReadByTypeResponse : public ControllerToHostPacket {

      public:
        static const Packet::Type initial_type() {
          return Packet::Type::HCI;
        };

        static const uint16_t initial_packet_code() {
          return Format::HCI::AttributeCode::ReadByTypeResponse;
        };

        static const uint16_t final_packet_code() {
          return initial_packet_code();
        };

        ReadByTypeResponse();

        void unserialize(HCIFormatExtractor& extractor) override;

        const std::string stringify() const override;

        inline std::vector<Format::HCI::Characteristic> get_characteristics() const {
          return m_characteristics;
        };

        inline uint16_t get_last_ending_handle() const {
          return m_last_ending_handle;
        };

      private:
        std::vector<Format::HCI::Characteristic> m_characteristics;
        uint16_t m_last_ending_handle;

      };

    }

  }

}

#endif //BABLE_CENTRAL_READBYTYPERESPONSE_HPP
