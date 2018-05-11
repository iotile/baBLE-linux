#include "GetConnectedDevices.hpp"

using namespace std;

namespace Packet::Commands {

  GetConnectedDevices::GetConnectedDevices(Packet::Type initial_type, Packet::Type translated_type)
      : CommandPacket(initial_type, translated_type) {};

  void GetConnectedDevices::import(AsciiFormatExtractor& extractor) {
    CommandPacket::import(extractor);

    try {
      m_controller_id = static_cast<uint16_t>(stoi(extractor.get()));

    } catch (const Exceptions::WrongFormatException& err) {
      throw Exceptions::InvalidCommandException("Missing arguments for 'GetConnectedDevices' packet. Usage: <command_code>,<controller_id>");
    } catch (const std::bad_cast& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'GetConnectedDevices' packet. Can't cast.");
    } catch (const std::invalid_argument& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'GetConnectedDevices' packet.");
    }
  };

  void GetConnectedDevices::import(FlatbuffersFormatExtractor& extractor) {
    CommandPacket::import(extractor);
    auto payload = extractor.get_payload<const Schemas::GetControllerInfo*>(Schemas::Payload::GetControllerInfo);

    m_controller_id = payload->controller_id();
  };

  void GetConnectedDevices::import(MGMTFormatExtractor& extractor) {
    CommandPacket::import(extractor);

    if (m_native_status == 0) {
      auto m_num_connections = extractor.get_value<uint16_t>();
      m_devices.reserve(m_num_connections);

      std::array<uint8_t, 6> current_address{};
      uint8_t current_address_type;

      for (size_t i = 0; i < m_num_connections; i++) {
        current_address = extractor.get_array<uint8_t, 6>();
        m_devices.push_back(AsciiFormat::format_bd_address(current_address));

        current_address_type = extractor.get_value<uint8_t>();
      }
    }
  };

  vector<uint8_t> GetConnectedDevices::serialize(AsciiFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder
        .set_name("GetConnectedDevices")
        .add("Num. of connections", m_devices.size())
        .add("Devices", m_devices);

    return builder.build();
  };

  vector<uint8_t> GetConnectedDevices::serialize(FlatbuffersFormatBuilder& builder) const {
    CommandPacket::serialize(builder);

    std::vector<flatbuffers::Offset<flatbuffers::String>> devices_fb_string;
    devices_fb_string.reserve(m_devices.size());
    for (auto& value : m_devices) {
      devices_fb_string.push_back(builder.CreateString(value));
    }

    auto devices = builder.CreateVector(devices_fb_string);
    auto payload = Schemas::CreateGetConnectedDevices(builder, m_controller_id, devices);

    return builder.build(payload, Schemas::Payload::GetConnectedDevices, m_native_class, m_status, m_native_status);
  }

  vector<uint8_t> GetConnectedDevices::serialize(MGMTFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    return builder.build();
  };

}