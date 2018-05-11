#include "Disconnect.hpp"

using namespace std;

namespace Packet::Commands {

  Disconnect::Disconnect(Packet::Type initial_type, Packet::Type translated_type)
      : CommandPacket(initial_type, translated_type) {
    m_address_type = 0;
  };

  void Disconnect::import(AsciiFormatExtractor& extractor) {
    CommandPacket::import(extractor);

    try {
      m_controller_id = static_cast<uint16_t>(stoi(extractor.get()));
      m_address_type = static_cast<uint8_t>(stoi(extractor.get()));

      string address_str = extractor.get();
      string item;
      istringstream address_stream(address_str);
      for (uint8_t& item_address : m_address) {
        if (!getline(address_stream, item, ':')) {
          throw Exceptions::InvalidCommandException("Can't parse <address> argument for 'Disconnect' packet.");
        }
        item_address = static_cast<uint8_t>(stoi(item));
      }

    } catch (const runtime_error& err) {
      throw Exceptions::InvalidCommandException("Missing arguments for 'Disconnect' packet. Usage: <command_code>,<controller_id>,<address_type>,<address>");
    } catch (const bad_cast& err) {
      throw Exceptions::InvalidCommandException("Invalid address argument for 'Disconnect' packet.");
    }
  };

  void Disconnect::import(FlatbuffersFormatExtractor& extractor) {
    CommandPacket::import(extractor);
    auto payload = extractor.get_payload<const Schemas::Disconnect*>(Schemas::Payload::Disconnect);

    m_controller_id = payload->controller_id();
    m_address_type = payload->address_type();

    auto raw_fb_address = payload->address();
    if (raw_fb_address->size() != m_address.size()) {
      throw Exceptions::InvalidCommandException("Given address is not valid: size different from expected.");
    }
    copy(raw_fb_address->begin(), raw_fb_address->end(), m_address.begin());
  };

  void Disconnect::import(MGMTFormatExtractor& extractor) {
    CommandPacket::import(extractor);

    if (m_native_status == 0){
      m_address = extractor.get_array<uint8_t, 6>();
      m_address_type = extractor.get_value<uint8_t>();
    }
  };

  vector<uint8_t> Disconnect::serialize(AsciiFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder
        .set_name("Disconnect")
        .add("Address type", m_address_type)
        .add("Address", m_address);

    return builder.build();
  };

  vector<uint8_t> Disconnect::serialize(FlatbuffersFormatBuilder& builder) const {
    CommandPacket::serialize(builder);

    auto address = builder.CreateVector(m_address.data(), m_address.size());

    auto payload = Schemas::CreateDisconnect(builder, m_controller_id, address, m_address_type);

    return builder.build(payload, Schemas::Payload::Disconnect, m_native_class, m_status, m_native_status);
  }

  vector<uint8_t> Disconnect::serialize(MGMTFormatBuilder& builder) const {
    CommandPacket::serialize(builder);

    builder
        .add(m_address)
        .add(m_address_type);
    return builder.build();
  };

}
