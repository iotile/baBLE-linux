#ifndef BABLE_LINUX_WRITE_HPP
#define BABLE_LINUX_WRITE_HPP

#include "../CommandPacket.hpp"

namespace Packet::Commands {

  class Write : public CommandPacket<Write> {

  public:
    static const uint16_t packet_code(Packet::Type type) {
      switch(type) {
        case Packet::Type::MGMT:
          throw std::invalid_argument("'Write' packet is not compatible with MGMT protocol.");

        case Packet::Type::HCI:
          return Format::HCI::AttributeCode::WriteRequest;

        case Packet::Type::ASCII:
          return Format::Ascii::CommandCode::Write;

        case Packet::Type::FLATBUFFERS:
          return static_cast<uint16_t>(Schemas::Payload::Write);

        case Packet::Type::NONE:
          return 0;
      }
    };

    Write(Packet::Type initial_type, Packet::Type translated_type);

    void unserialize(AsciiFormatExtractor& extractor) override;
    void unserialize(FlatbuffersFormatExtractor& extractor) override;
    void unserialize(HCIFormatExtractor& extractor) override;

    std::vector<uint8_t> serialize(AsciiFormatBuilder& builder) const override;
    std::vector<uint8_t> serialize(FlatbuffersFormatBuilder& builder) const override;
    std::vector<uint8_t> serialize(HCIFormatBuilder& builder) const override;

    std::vector<ResponseId> expected_response_ids() override {
      if (!m_response_received) {
        return {
          ResponseId{current_type(), m_controller_id, m_connection_handle, Format::HCI::AttributeCode::WriteResponse}
        };

      } else {
        return {};
      }
    };

  private:
    uint16_t m_connection_handle;
    uint16_t m_attribute_handle;
    std::vector<uint8_t> m_data_to_write;

  };

}

#endif //BABLE_LINUX_WRITE_HPP
