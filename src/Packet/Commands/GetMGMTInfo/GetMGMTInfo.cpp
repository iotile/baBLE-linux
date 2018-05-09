#include "GetMGMTInfo.hpp"

namespace Packet::Commands {

  GetMGMTInfo::GetMGMTInfo(Packet::Type initial_type, Packet::Type translated_type)
  : CommandPacket(initial_type, translated_type) {
    m_version = 0;
    m_revision = 0;
  };

  void GetMGMTInfo::import(AsciiFormatExtractor& extractor) {
    CommandPacket::import(extractor);
  };

  void GetMGMTInfo::import(FlatbuffersFormatExtractor& extractor) {
    CommandPacket::import(extractor);
  };

  void GetMGMTInfo::import(MGMTFormatExtractor& extractor) {
    CommandPacket::import(extractor);

    if (m_native_status == 0) {
    m_version = extractor.get_value<uint8_t>();
    m_revision = extractor.get_value<uint16_t>();
    }
  };

  std::vector<uint8_t> GetMGMTInfo::serialize(AsciiFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    builder
      .set_name("GetMGMTInfo")
      .add("Version", m_version)
      .add("Revision", m_revision);

    return builder.build();
  };

  std::vector<uint8_t> GetMGMTInfo::serialize(FlatbuffersFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    auto payload = Schemas::CreateGetMGMTInfo(builder, m_version, m_revision);

    return builder.build(payload, Schemas::Payload::GetMGMTInfo, m_native_class, m_status, m_native_status);
  }

  std::vector<uint8_t> GetMGMTInfo::serialize(MGMTFormatBuilder& builder) const {
    CommandPacket::serialize(builder);
    return builder.build();
  };

}