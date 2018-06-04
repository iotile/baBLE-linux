#include "DisconnectRequest.hpp"
#include "../../../../Exceptions/InvalidCommand/InvalidCommandException.hpp"
#include "../../../../utils/string_formats.hpp"

using namespace std;

namespace Packet {

  namespace Commands {

    DisconnectRequest::DisconnectRequest(Packet::Type initial_type, Packet::Type translated_type)
        : RequestPacket(initial_type, translated_type) {
      m_id = Packet::Id::DisconnectRequest;
      m_address_type = 0;
    }

    void DisconnectRequest::unserialize(FlatbuffersFormatExtractor& extractor) {
      auto payload = extractor.get_payload<const BaBLE::Disconnect*>();

      m_address_type = payload->address_type();
      m_address = payload->address()->str();

      string item;
      istringstream address_stream(m_address);
      for (auto it = m_raw_address.rbegin(); it != m_raw_address.rend(); ++it) {
        if (!getline(address_stream, item, ':')) {
          throw Exceptions::InvalidCommandException("Can't parse 'address' argument for 'Disconnect' packet.",
                                                    m_uuid_request);
        }
        *it = Utils::string_to_number<uint8_t>(item, 16);
      }
    };

    vector<uint8_t> DisconnectRequest::serialize(MGMTFormatBuilder& builder) const {
      RequestPacket::serialize(builder);

      builder
          .add(m_raw_address)
          .add(m_address_type);
      return builder.build();
    }

    const std::string DisconnectRequest::stringify() const {
      stringstream result;

      result << "<DisconnectRequest> "
             << AbstractPacket::stringify() << ", "
             << "Address type: " << to_string(m_address_type) << ", "
             << "Address: " << m_address;

      return result.str();
    }

  }

}
