#include <sstream>
#include "utils/string_formats.hpp"
#include "SetGATTTable.hpp"
#include "Transport/Socket/HCI/HCISocket.hpp"

using namespace std;

namespace Packet {

  namespace Control {

    SetGATTTable::SetGATTTable()
        : HostOnlyPacket(Packet::Id::SetGATTTable, initial_packet_code()) {}

    void SetGATTTable::unserialize(FlatbuffersFormatExtractor& extractor) {
      auto payload = extractor.get_payload<const BaBLE::SetGATTTable*>();

      auto raw_services = payload->services();
      auto raw_characteristics = payload->characteristics();

      for (uint32_t i = 0; i < raw_services->Length(); i++) {
        auto raw_service = raw_services->Get(i);

        Format::HCI::Service service{};
        service.handle = raw_service->handle();
        service.group_end_handle = raw_service->group_end_handle();
        service.uuid =  Utils::extract_uuid(raw_service->uuid()->str());

        m_services.push_back(service);
      }

      for (uint32_t i = 0; i < raw_characteristics->Length(); i++) {
        auto raw_characteristic = raw_characteristics->Get(i);

        uint8_t properties = 0;
        if (raw_characteristic->indicate()) properties |= 1 << 5;
        if (raw_characteristic->notify()) properties |= 1 << 4;
        if (raw_characteristic->write()) properties |= 1 << 3;
        if (raw_characteristic->read()) properties |= 1 << 1;
        if (raw_characteristic->broadcast()) properties |= 1 << 0;

        uint16_t configuration = 0;
        if (raw_characteristic->indication_enabled()) configuration |= 1 << 1;
        if (raw_characteristic->notification_enabled()) configuration |= 1 << 0;

        vector<uint8_t> const_value;
        if (raw_characteristic->const_value() != nullptr) {
          const_value.assign(raw_characteristic->const_value()->begin(), raw_characteristic->const_value()->end());
        }

        Format::HCI::Characteristic characteristic{};
        characteristic.handle = raw_characteristic->handle();
        characteristic.properties = properties;
        characteristic.value_handle = raw_characteristic->value_handle();
        characteristic.config_handle = raw_characteristic->config_handle();
        characteristic.configuration = configuration;
        characteristic.uuid = Utils::extract_uuid(raw_characteristic->uuid()->str());
        characteristic.const_value = const_value;

        m_characteristics.push_back(characteristic);
      }
    }

    vector<uint8_t> SetGATTTable::serialize(FlatbuffersFormatBuilder& builder) const {
      auto payload = BaBLE::CreateSetGATTTable(builder);

      return builder.build(payload, BaBLE::Payload::SetGATTTable);
    }

    void SetGATTTable::set_socket(AbstractSocket* socket) {
      auto hci_socket = dynamic_cast<HCISocket*>(socket);
      if (hci_socket == nullptr) {
        throw Exceptions::BaBLEException(BaBLE::StatusCode::Failed, "Can't downcast socket to HCISocket packet");
      }

      hci_socket->set_gatt_table(m_services, m_characteristics);
    }

    const string SetGATTTable::stringify() const {
      stringstream result;

      result << "<SetGATTTable> "
             << AbstractPacket::stringify() << ", "
             << "Services: ";

      for (auto& service : m_services) {
        result << "{ Handle: " << to_string(service.handle) << ", "
               << "Group end handle: " << to_string(service.group_end_handle) << ", "
               << "UUID: " << Utils::format_uuid(service.uuid) << "} " << ", ";
      }

      result << "Characteristics: ";
      for (auto it = m_characteristics.begin(); it != m_characteristics.end(); ++it) {
        result << "{ Handle: " << to_string(it->handle) << ", "
               << "Properties: " << to_string(it->properties) << ", "
               << "Value handle: " << to_string(it->value_handle) << ", "
               << "Config handle: " << to_string(it->config_handle) << ", "
               << "Configuration: " << to_string(it->configuration) << ", "
               << "Constant value: " << Utils::format_bytes_array(it->const_value) << ", "
               << "UUID: " << Utils::format_uuid(it->uuid) << "} ";
        if (next(it) != m_characteristics.end()) {
          result << ", ";
        }
      }

      return result.str();
    }

  }

}