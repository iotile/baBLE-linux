#include "SetConnectableRequest.hpp"
#include "../../../../Exceptions/InvalidCommand/InvalidCommandException.hpp"

using namespace std;

namespace Packet {

  namespace Commands {

    SetConnectableRequest::SetConnectableRequest(Packet::Type initial_type, Packet::Type translated_type)
        : RequestPacket(initial_type, translated_type) {
      m_id = Packet::Id::SetConnectableRequest;
      m_state = false;
    }

    void SetConnectableRequest::unserialize(AsciiFormatExtractor& extractor) {
      try {
        m_state = AsciiFormat::string_to_number<bool>(extractor.get_string());

      } catch (const Exceptions::WrongFormatException& err) {
        throw Exceptions::InvalidCommandException("Invalid arguments for 'SetConnectable' packet."
                                                  "Usage: <uuid>,<command_code>,<controller_id>,"
                                                  "<state>", m_uuid_request);
      }
    }

    void SetConnectableRequest::unserialize(FlatbuffersFormatExtractor& extractor) {
      auto payload = extractor.get_payload<const BaBLE::SetConnectable*>();

      m_state = payload->state();
    }

    vector<uint8_t> SetConnectableRequest::serialize(AsciiFormatBuilder& builder) const {
      RequestPacket::serialize(builder);
      builder
          .set_name("SetConnectable")
          .add("State", m_state);

      return builder.build();
    }

    vector<uint8_t> SetConnectableRequest::serialize(MGMTFormatBuilder& builder) const {
      RequestPacket::serialize(builder);
      builder.add(m_state);

      return builder.build();
    }

  }

}
