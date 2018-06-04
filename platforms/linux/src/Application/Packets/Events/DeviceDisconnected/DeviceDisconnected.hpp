#ifndef BABLE_LINUX_DEVICEDISCONNECTED_HPP
#define BABLE_LINUX_DEVICEDISCONNECTED_HPP

#include "../EventPacket.hpp"

namespace Packet {

  namespace Events {

    class DeviceDisconnected : public EventPacket<DeviceDisconnected> {

    public:
      static const uint16_t packet_code(Packet::Type type) {
        switch (type) {
          case Packet::Type::MGMT:
            throw std::invalid_argument("'DeviceDisconnected' packet not implemented with MGMT protocol.");

          case Packet::Type::HCI:
            return Format::HCI::EventCode::DisconnectComplete;

          case Packet::Type::FLATBUFFERS:
            return static_cast<uint16_t>(BaBLE::Payload::DeviceDisconnected);

          case Packet::Type::NONE:
            return 0;
        }
      };

      DeviceDisconnected(Packet::Type initial_type, Packet::Type final_type);

      void unserialize(HCIFormatExtractor& extractor) override;

      std::vector<uint8_t> serialize(FlatbuffersFormatBuilder& builder) const override;

      const std::string stringify() const override;

    private:
      uint8_t m_raw_reason;
      std::string m_reason;
    };

  }

}
#endif //BABLE_LINUX_DEVICEDISCONNECTED_HPP
