#include "SetDiscoverable.hpp"

using namespace std;

namespace Packet::Commands {

  SetDiscoverable::SetDiscoverable(Packet::Type initial_type, Packet::Type translated_type)
      : CommandPacket(initial_type, translated_type) {
    m_state = 0;
    m_timeout = 0;
  }

  void SetDiscoverable::import(AsciiFormatExtractor& extractor) {
    CommandPacket::import(extractor);

    try {
      m_controller_id = static_cast<uint16_t>(stoi(extractor.get()));
      m_state = static_cast<uint8_t>(stoi(extractor.get()));
      m_timeout = static_cast<uint16_t>(stoi(extractor.get()));

      if (m_state < 0 || m_state > 2) {
        throw Exceptions::InvalidCommandException("Invalid value for state for 'SetDiscoverable' packet.");
      }
      if (m_state == 2 && m_timeout == 0) {
        throw Exceptions::InvalidCommandException("Timeout is required if state = 2 for 'SetDiscoverable' packet.");
      }

    } catch (const Exceptions::WrongFormatException& err) {
      throw Exceptions::InvalidCommandException("Missing arguments for 'SetDiscoverable' packet. Usage: <command_code>,<controller_id>,<state>");
    } catch (const std::bad_cast& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'SetDiscoverable' packet. Can't cast.");
    } catch (const std::invalid_argument& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'SetDiscoverable' packet.");
    }
  }

  void SetDiscoverable::import(FlatbuffersFormatExtractor& extractor) {
    CommandPacket::import(extractor);
    auto payload = extractor.get_payload<const Schemas::SetDiscoverable*>(Schemas::Payload::SetDiscoverable);

    m_controller_id = payload->controller_id();
    m_state = static_cast<uint8_t>(payload->state());
    m_timeout = payload->timeout();
  }

  void SetDiscoverable::import(MGMTFormatExtractor& extractor) {
    CommandPacket::import(extractor);

    if (m_native_status == 0){
      auto m_current_settings = extractor.get_value<uint32_t>();
      m_state = static_cast<uint8_t>(m_current_settings & 1 << 3);
    }
  }

  vector<uint8_t> SetDiscoverable::serialize(AsciiFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder
        .set_name("SetDiscoverable")
        .add("State", m_state)
        .add("Timeout", m_timeout);

    return builder.build();
  }

  vector<uint8_t> SetDiscoverable::serialize(FlatbuffersFormatBuilder& builder) const {
    CommandPacket::serialize(builder);

    auto payload = Schemas::CreateSetDiscoverable(builder, m_controller_id, m_state, m_timeout);

    return builder.build(payload, Schemas::Payload::SetDiscoverable, m_native_class, m_status, m_native_status);
  }

  vector<uint8_t> SetDiscoverable::serialize(MGMTFormatBuilder& builder) const {
    CommandPacket::serialize(builder);

    builder
        .add(m_state)
        .add(m_timeout);
    return builder.build();
  }

}
