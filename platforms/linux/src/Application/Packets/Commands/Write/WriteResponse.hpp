#ifndef BABLE_LINUX_WRITERESPONSE_HPP
#define BABLE_LINUX_WRITERESPONSE_HPP

#include "../../Base/ControllerToHostPacket.hpp"

namespace Packet {

  namespace Commands {

    class WriteResponse : public ControllerToHostPacket {

    public:
      static const Packet::Type initial_type() {
        return Packet::Type::HCI;
      };

      static const uint16_t initial_packet_code() {
        return Format::HCI::AttributeCode::WriteResponse;
      };

      static const uint16_t final_packet_code() {
        return static_cast<uint16_t>(BaBLE::Payload::Write);
      };

      WriteResponse();

      void unserialize(HCIFormatExtractor& extractor) override;
      std::vector<uint8_t> serialize(FlatbuffersFormatBuilder& builder) const override;

      const std::string stringify() const override;

      inline void set_attribute_handle(uint16_t attribute_handle) {
        m_attribute_handle = attribute_handle;
      };

    private:
      uint16_t m_attribute_handle;

    };

  }

}

#endif //BABLE_LINUX_WRITERESPONSE_HPP