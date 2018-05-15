#include "StartScan.hpp"

using namespace std;

namespace Packet::Commands {

  StartScan::StartScan(Packet::Type initial_type, Packet::Type translated_type)
  : CommandPacket(initial_type, translated_type) {
    m_address_type = 0;
  };

  void StartScan::unserialize(AsciiFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);

    try {
      m_address_type = static_cast<uint8_t>(stoi(extractor.get()));

    } catch (const Exceptions::WrongFormatException& err) {
      throw Exceptions::InvalidCommandException("Missing arguments for 'StartScan' packet. Usage: <command_code>,<controller_id>,<address_type>");
    } catch (const std::bad_cast& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'StartScan' packet. Can't cast.");
    } catch (const std::invalid_argument& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'StartScan' packet.");
    }
  };

  void StartScan::unserialize(FlatbuffersFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);
    auto payload = extractor.get_payload<const Schemas::StartScan*>(Schemas::Payload::StartScan);

    m_address_type = payload->address_type();
  };

  void StartScan::unserialize(MGMTFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);

    if (m_native_status == 0){
      m_address_type = extractor.get_value<uint8_t>();
    }
  };

  vector<uint8_t> StartScan::serialize(AsciiFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder
      .set_name("StartScan")
      .add("Address type", m_address_type);

    return builder.build();
  };

  vector<uint8_t> StartScan::serialize(FlatbuffersFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    auto payload = Schemas::CreateStartScan(builder, m_address_type);

    return builder.build(m_controller_id, payload, Schemas::Payload::StartScan, m_native_class, m_status, m_native_status);
  }

  vector<uint8_t> StartScan::serialize(MGMTFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder.add(m_address_type);
    return builder.build();
  };

}