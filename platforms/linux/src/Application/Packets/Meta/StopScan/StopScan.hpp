#ifndef BABLE_STOPSCAN_HPP
#define BABLE_STOPSCAN_HPP

#include "Application/Packets/Base/HostOnlyPacket.hpp"
#include "Application/Packets/Commands/SetScanParameters/SetScanParameters.hpp"
#include "Application/Packets/Commands/SetScanEnable/SetScanEnable.hpp"

namespace Packet {

  namespace Meta {

    class StopScan : public HostOnlyPacket {

    public:
      static const uint16_t initial_packet_code() {
        return static_cast<uint16_t>(BaBLE::Payload::StopScan);
      };

      StopScan();

      void unserialize(FlatbuffersFormatExtractor& extractor) override;

      std::vector<uint8_t> serialize(FlatbuffersFormatBuilder& builder) const override;
      std::vector<uint8_t> serialize(HCIFormatBuilder& builder) const override;

      const std::string stringify() const override;

      void prepare(const std::shared_ptr<PacketRouter>& router) override;
      std::shared_ptr<AbstractPacket> on_set_scan_enable_response_received(const std::shared_ptr<PacketRouter>& router,
                                                                           const std::shared_ptr<AbstractPacket>& packet);

    private:
      bool m_waiting_response;

      std::shared_ptr<Commands::SetScanParameters> m_set_scan_params_packet;
      std::shared_ptr<Commands::SetScanEnable> m_set_scan_enable_packet;

    };

  }

}

#endif //BABLE_STOPSCAN_HPP
