#ifndef BABLE_LINUX_EXIT_HPP
#define BABLE_LINUX_EXIT_HPP

#include "../../../AbstractPacket.hpp"

namespace Packet {

  namespace Control {

    class Exit : public AbstractPacket {

    public:
      static const uint16_t packet_code(Packet::Type type) {
        switch (type) {
          case Packet::Type::MGMT:
            throw std::invalid_argument("'Exit' packet is not compatible with MGMT protocol.");

          case Packet::Type::HCI:
            throw std::invalid_argument("'Exit' packet is not compatible with HCI protocol.");

          case Packet::Type::FLATBUFFERS:
            return static_cast<uint16_t>(BaBLE::Payload::Exit);

          case Packet::Type::NONE:
            return 0;
        }
      };

      Exit(Packet::Type initial_type, Packet::Type final_type);

      void unserialize(FlatbuffersFormatExtractor& extractor) override;

      std::vector<uint8_t> serialize(FlatbuffersFormatBuilder& builder) const override;

      const std::string stringify() const override;

    };

  }

}

#endif //BABLE_LINUX_EXIT_HPP
