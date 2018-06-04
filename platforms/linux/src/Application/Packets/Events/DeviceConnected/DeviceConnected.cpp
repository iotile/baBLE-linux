#include "DeviceConnected.hpp"
#include "../../../../Log/Log.hpp"
#include "../../../../utils/string_formats.hpp"

using namespace std;

namespace Packet {

  namespace Events {

    DeviceConnected::DeviceConnected(Packet::Type initial_type, Packet::Type final_type)
        : EventPacket(initial_type, final_type) {
      m_id = Packet::Id::DeviceConnected;
      m_address_type = 0x00;
    }

    void DeviceConnected::unserialize(HCIFormatExtractor& extractor) {
      set_status(extractor.get_value<uint8_t>());
      m_connection_id = extractor.get_value<uint16_t>();
      auto role = extractor.get_value<uint8_t>();
      m_address_type = static_cast<uint8_t>(extractor.get_value<uint8_t>() + 1);
      m_raw_address = extractor.get_array<uint8_t, 6>();

      m_address = Utils::format_bd_address(m_raw_address);
    }

    vector<uint8_t> DeviceConnected::serialize(FlatbuffersFormatBuilder& builder) const {
      auto address = builder.CreateString(m_address);

      auto payload = BaBLE::CreateDeviceConnected(
          builder,
          m_connection_id,
          address,
          m_address_type
      );

      return builder.build(payload, BaBLE::Payload::DeviceConnected);
    }

    const std::string DeviceConnected::stringify() const {
      stringstream result;

      result << "<DeviceConnected> "
             << AbstractPacket::stringify() << ", "
             << "Address: " << m_address << ", "
             << "Address type: " << to_string(m_address_type);

      return result.str();
    }

  }

}