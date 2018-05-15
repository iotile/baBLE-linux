#include "StopScan.hpp"

using namespace std;

namespace Packet::Commands {

  StopScan::StopScan(Packet::Type initial_type, Packet::Type translated_type)
  : CommandPacket(initial_type, translated_type) {
    m_address_type = 0;
  }

  void StopScan::unserialize(AsciiFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);

    try {
      m_address_type = static_cast<uint8_t>(stoi(extractor.get()));

    } catch (const Exceptions::WrongFormatException& err) {
      throw Exceptions::InvalidCommandException("Missing arguments for 'StopScan' packet. Usage: <command_code>,<controller_id>,<address_type>");
    } catch (const std::bad_cast& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'StopScan' packet. Can't cast.");
    } catch (const std::invalid_argument& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'StopScan' packet.");
    }
  };

  void StopScan::unserialize(FlatbuffersFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);
    auto payload = extractor.get_payload<const Schemas::StartScan*>(Schemas::Payload::StartScan);

    m_address_type = payload->address_type();
  };

  void StopScan::unserialize(MGMTFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);

    if (m_native_status == 0) {
    m_address_type = extractor.get_value<uint8_t>();
    }
  };

  vector<uint8_t> StopScan::serialize(AsciiFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder
    .set_name("StopScan")
    .add("Address type", m_address_type);

    return builder.build();
  };

  vector<uint8_t> StopScan::serialize(FlatbuffersFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    auto payload = Schemas::CreateStopScan(builder, m_address_type);

    return builder.build(m_controller_id, payload, Schemas::Payload::StopScan, m_native_class, m_status, m_native_status);
  }

  vector<uint8_t> StopScan::serialize(MGMTFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder.add(m_address_type);
    return builder.build();
  };

}
