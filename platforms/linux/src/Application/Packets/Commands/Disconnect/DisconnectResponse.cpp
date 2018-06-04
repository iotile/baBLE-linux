#include "DisconnectResponse.hpp"
#include "../../../../utils/string_formats.hpp"

using namespace std;

namespace Packet {

  namespace Commands {

    DisconnectResponse::DisconnectResponse(Packet::Type initial_type, Packet::Type translated_type)
        : ResponsePacket(initial_type, translated_type) {
      m_id = Packet::Id::DisconnectResponse;
      m_address_type = 0;
    }

    void DisconnectResponse::unserialize(MGMTFormatExtractor& extractor) {
      ResponsePacket::unserialize(extractor);

      if (m_status == BaBLE::StatusCode::Success) {
        m_raw_address = extractor.get_array<uint8_t, 6>();
        m_address = Utils::format_bd_address(m_raw_address);
        m_address_type = extractor.get_value<uint8_t>();
      }
    }

    vector<uint8_t> DisconnectResponse::serialize(FlatbuffersFormatBuilder& builder) const {
      auto address = builder.CreateString(m_address);

      auto payload = BaBLE::CreateDisconnect(builder, address, m_address_type);

      return builder.build(payload, BaBLE::Payload::Disconnect);
    }

    const std::string DisconnectResponse::stringify() const {
      stringstream result;

      result << "<DisconnectResponse> "
             << AbstractPacket::stringify() << ", "
             << "Address type: " << to_string(m_address_type) << ", "
             << "Address: " << m_address;

      return result.str();
    }

  }

}
