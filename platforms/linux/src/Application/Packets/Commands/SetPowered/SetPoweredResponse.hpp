#ifndef BABLE_LINUX_SETPOWEREDRESPONSE_HPP
#define BABLE_LINUX_SETPOWEREDRESPONSE_HPP

#include "../ResponsePacket.hpp"

namespace Packet {

  namespace Commands {

    class SetPoweredResponse : public ResponsePacket<SetPoweredResponse> {

    public:
      static const uint16_t packet_code(Packet::Type type) {
        switch (type) {
          case Packet::Type::MGMT:
            return Format::MGMT::CommandCode::SetPowered;

          case Packet::Type::HCI:
            throw std::invalid_argument("'SetPowered' packet is not compatible with HCI protocol.");

          case Packet::Type::FLATBUFFERS:
            return static_cast<uint16_t>(BaBLE::Payload::SetPowered);

          case Packet::Type::NONE:
            return 0;
        }
      };

      SetPoweredResponse(Packet::Type initial_type, Packet::Type final_type);

      void unserialize(MGMTFormatExtractor& extractor) override;

      std::vector<uint8_t> serialize(FlatbuffersFormatBuilder& builder) const override;

      const std::string stringify() const override;

    private:
      bool m_state;

    };

  }

}

#endif //BABLE_LINUX_SETPOWEREDRESPONSE_HPP
