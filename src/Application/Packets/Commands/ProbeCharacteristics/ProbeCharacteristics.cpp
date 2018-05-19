#include "ProbeCharacteristics.hpp"

using namespace std;

namespace Packet::Commands {

  ProbeCharacteristics::ProbeCharacteristics(Packet::Type initial_type, Packet::Type translated_type)
      : CommandPacket(initial_type, translated_type) {
    m_starting_handle = 0x0001;
    m_ending_handle = 0xFFFF;
    m_waiting_characteristics = true;
  }

  void ProbeCharacteristics::unserialize(AsciiFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);

    try {
      m_connection_handle = static_cast<uint16_t>(stoi(extractor.get()));

    } catch (const Exceptions::WrongFormatException& err) {
      throw Exceptions::InvalidCommandException("Missing arguments for 'ProbeCharacteristics' packet."
                                                "Usage: <uuid>,<command_code>,<controller_id>,<connection_handle>", m_uuid_request);
    } catch (const bad_cast& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'ProbeCharacteristics' packet. Can't cast.", m_uuid_request);
    } catch (const std::invalid_argument& err) {
      throw Exceptions::InvalidCommandException("Invalid arguments for 'ProbeCharacteristics' packet.", m_uuid_request);
    }
  }

  void ProbeCharacteristics::unserialize(FlatbuffersFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);
    auto payload = extractor.get_payload<const Schemas::ProbeCharacteristics*>(Schemas::Payload::ProbeCharacteristics);

    m_connection_handle = payload->connection_handle();
  }

  void ProbeCharacteristics::unserialize(HCIFormatExtractor& extractor) {
    CommandPacket::unserialize(extractor);

    uint16_t data_length = extractor.get_data_length();

    if (data_length <= 0) {
      throw Exceptions::WrongFormatException("Receive HCI 'ProbeCharacteristics' response with no data.");
    }

    auto attribute_length = extractor.get_value<uint8_t>();
    data_length -= 1; // To remove attribute_length from remaining data_length

    size_t num_characteristics = data_length / attribute_length;
    m_characteristics.reserve(m_characteristics.size() + num_characteristics);

    while(data_length >= attribute_length) {
      Format::HCI::Characteristic characteristic{};
      characteristic.handle = extractor.get_value<uint16_t>();
      characteristic.properties = extractor.get_value<uint8_t>();
      characteristic.value_handle = extractor.get_value<uint16_t>();
      characteristic.uuid = extractor.get_vector<uint8_t>(attribute_length
                                                              - sizeof(characteristic.handle)
                                                              - sizeof(characteristic.properties)
                                                              - sizeof(characteristic.value_handle));

      m_characteristics.push_back(characteristic);
      m_ending_handle = characteristic.value_handle;
      data_length -= attribute_length;
    }

    if (m_ending_handle == 0xFFFF) {
      m_waiting_characteristics = false;
    } else {
      m_waiting_characteristics = true;
      m_starting_handle = static_cast<uint16_t>(m_ending_handle + 1);
      m_ending_handle = 0xFFFF;
    }
  }

  vector<uint8_t> ProbeCharacteristics::serialize(AsciiFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder
        .set_name("ProbeCharacteristics")
        .add("Connection handle", m_connection_handle)
        .add("Starting handle", m_starting_handle)
        .add("Ending handle", m_ending_handle);

    for (auto& characteristic : m_characteristics) {
      builder
          .add("Handle", characteristic.handle)
          .add("Properties", characteristic.properties)
          .add("Value handle", characteristic.value_handle)
          .add("UUID", characteristic.uuid);
    }

    return builder.build();
  }

  vector<uint8_t> ProbeCharacteristics::serialize(FlatbuffersFormatBuilder& builder) const {
    CommandPacket::serialize(builder);

    std::vector<flatbuffers::Offset<Schemas::Characteristic>> characteristics;
    characteristics.reserve(m_characteristics.size());

    for (auto& characteristic : m_characteristics) {
      auto uuid = builder.CreateString(AsciiFormat::format_uuid(characteristic.uuid));

      bool indicate = (characteristic.properties & 1 << 5) > 0;
      bool notify = (characteristic.properties & 1 << 4) > 0;
      bool write = (characteristic.properties & 1 << 3) > 0 | (characteristic.properties & 1 << 2) > 0;
      bool read = (characteristic.properties & 1 << 1) > 0;
      bool broadcast = (characteristic.properties & 1) > 0;

      auto characteristic_offset = Schemas::CreateCharacteristic(
          builder,
          characteristic.handle,
          characteristic.value_handle,
          indicate,
          notify,
          write,
          read,
          broadcast,
          uuid
      );
      characteristics.push_back(characteristic_offset);
    }

    auto characteristics_vector = builder.CreateVector(characteristics);
    auto payload = Schemas::CreateProbeCharacteristics(builder, m_connection_handle, characteristics_vector);

    return builder.build(payload, Schemas::Payload::ProbeCharacteristics);
  }

  vector<uint8_t> ProbeCharacteristics::serialize(HCIFormatBuilder& builder) const {
    CommandPacket::serialize(builder);

    auto uuid = static_cast<uint16_t>(Format::HCI::UUID::GattCharacteristicDeclaration);
    builder
        .set_connection_handle(m_connection_handle)
        .add(m_starting_handle)
        .add(m_ending_handle)
        .add(uuid);

    return builder.build();
  }

  void ProbeCharacteristics::translate() {
    if (!m_response_received | !m_waiting_characteristics) {
      AbstractPacket::translate();
    }
  }

  uint64_t ProbeCharacteristics::expected_response_uuid() {
    if (!m_response_received | m_waiting_characteristics) {
      return Packet::AbstractPacket::compute_uuid(
          m_controller_id,
          Format::HCI::AttributeCode::ReadByTypeResponse
      );
    } else {
      return 0;
    }
  }
}