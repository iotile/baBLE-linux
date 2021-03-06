#ifndef BABLE_REGISTRATION_CPP
#define BABLE_REGISTRATION_CPP

#include "Application/PacketBuilder/PacketBuilder.hpp"
#include "Application/Packets/Commands/Disconnect/Disconnect.hpp"
#include "Application/Packets/Commands/SetPowered/SetPoweredRequest.hpp"
#include "Application/Packets/Commands/SetPowered/SetPoweredResponse.hpp"
#include "Application/Packets/Commands/SetDiscoverable/SetDiscoverableRequest.hpp"
#include "Application/Packets/Commands/SetDiscoverable/SetDiscoverableResponse.hpp"
#include "Application/Packets/Commands/SetConnectable/SetConnectableRequest.hpp"
#include "Application/Packets/Commands/SetConnectable/SetConnectableResponse.hpp"
#include "Application/Packets/Commands/GetControllersIds/GetControllersIdsRequest.hpp"
#include "Application/Packets/Commands/GetControllersIds/GetControllersIdsResponse.hpp"
#include "Application/Packets/Commands/GetControllerInfo/GetControllerInfoRequest.hpp"
#include "Application/Packets/Commands/GetControllerInfo/GetControllerInfoResponse.hpp"
#include "Application/Packets/Commands/ReadByGroupType/Central/ReadByGroupTypeResponse.hpp"
#include "Application/Packets/Commands/ReadByGroupType/Peripheral/ReadByGroupType.hpp"
#include "Application/Packets/Commands/ReadByType/Peripheral/ReadByTypeRequest.hpp"
#include "Application/Packets/Commands/ReadByType/Central/ReadByTypeResponse.hpp"
#include "Application/Packets/Commands/FindInformation/Peripheral/FindInformation.hpp"
#include "Application/Packets/Commands/FindByType/Peripheral/FindByType.hpp"
#include "Application/Packets/Commands/Read/Central/ReadRequest.hpp"
#include "Application/Packets/Commands/Read/Central/ReadResponse.hpp"
#include "Application/Packets/Commands/Read/Peripheral/ReadRequest.hpp"
#include "Application/Packets/Commands/Read/Peripheral/ReadResponse.hpp"
#include "Application/Packets/Commands/Write/Central/WriteRequest.hpp"
#include "Application/Packets/Commands/Write/Central/WriteResponse.hpp"
#include "Application/Packets/Commands/Write/Peripheral/WriteRequest.hpp"
#include "Application/Packets/Commands/Write/Peripheral/WriteResponse.hpp"
#include "Application/Packets/Commands/HandleValueNotification/NotificationReceived.hpp"
#include "Application/Packets/Commands/WriteWithoutResponse/Peripheral/WriteWithoutResponse.hpp"
#include "Application/Packets/Commands/WriteWithoutResponse/Central/WriteWithoutResponse.hpp"
#include "Application/Packets/Commands/GetConnectedDevices/GetConnectedDevices.hpp"
#include "Application/Packets/Commands/CreateConnection/CreateConnection.hpp"
#include "Application/Packets/Commands/CancelConnection/CancelConnectionRequest.hpp"
#include "Application/Packets/Commands/HandleValueNotification/EmitNotification.hpp"
#include "Application/Packets/Events/DeviceConnected/DeviceConnected.hpp"
#include "Application/Packets/Events/DeviceDisconnected/DeviceDisconnected.hpp"
#include "Application/Packets/Events/ControllerAdded/ControllerAdded.hpp"
#include "Application/Packets/Events/ControllerRemoved/ControllerRemoved.hpp"
#include "Application/Packets/Events/AdvertisingReport/AdvertisingReport.hpp"
#include "Application/Packets/Events/CommandComplete/CommandComplete.hpp"
#include "Application/Packets/Events/CommandStatus/CommandStatus.hpp"
#include "Application/Packets/Events/NumberOfCompletedPackets/NumberOfCompletedPackets.hpp"
#include "Application/Packets/Control/SetGATTTable/SetGATTTable.hpp"
#include "Application/Packets/Control/Exit/Exit.hpp"
#include "Application/Packets/Control/Ready/Ready.hpp"
#include "Application/Packets/Errors/ErrorResponse/ErrorResponse.hpp"
#include "Application/Packets/Meta/SetAdvertising/SetAdvertising.hpp"
#include "Application/Packets/Meta/StartScan/StartScan.hpp"
#include "Application/Packets/Meta/StopScan/StopScan.hpp"
#include "Application/Packets/Meta/GetControllersList/GetControllersList.hpp"
#include "Application/Packets/Meta/ProbeServices/ProbeServices.hpp"
#include "Application/Packets/Meta/ProbeCharacteristics/ProbeCharacteristics.hpp"

// MGMT
void register_mgmt_packets(PacketBuilder& mgmt_packet_builder) {
  mgmt_packet_builder
    .register_command<Packet::Commands::GetControllersIdsResponse>()
    .register_command<Packet::Commands::GetControllerInfoResponse>()
    .register_command<Packet::Commands::SetPoweredResponse>()
    .register_command<Packet::Commands::SetDiscoverableResponse>()
    .register_command<Packet::Commands::SetConnectableResponse>()
    .register_event<Packet::Events::ControllerAdded>()
    .register_event<Packet::Events::ControllerRemoved>()
    .set_ignored_packets({
      Format::MGMT::EventCode::ClassOfDeviceChanged,
      Format::MGMT::EventCode::LocalNameChanged,
      Format::MGMT::EventCode::NewSettings,
      Format::MGMT::EventCode::DeviceAdded,
      Format::MGMT::EventCode::DeviceConnected,
      Format::MGMT::EventCode::DeviceDisconnected,
      Format::MGMT::EventCode::DeviceRemoved,
      Format::MGMT::EventCode::Discovering,
      Format::MGMT::EventCode::NewConnectionParameter
    });
}

// HCI
void register_hci_packets(PacketBuilder& hci_packet_builder) {
  hci_packet_builder
    .register_command<Packet::Commands::Central::ReadResponse>()
    .register_command<Packet::Commands::Peripheral::ReadRequest>()
    .register_command<Packet::Commands::Central::WriteResponse>()
    .register_command<Packet::Commands::Peripheral::WriteRequest>()
    .register_command<Packet::Commands::Peripheral::WriteWithoutResponse>()
    .register_command<Packet::Commands::Central::NotificationReceived>()
    .register_command<Packet::Commands::Central::ReadByGroupTypeResponse>()
    .register_command<Packet::Commands::Peripheral::ReadByGroupType>()
    .register_command<Packet::Commands::Central::ReadByTypeResponse>()
    .register_command<Packet::Commands::Peripheral::ReadByTypeRequest>()
    .register_command<Packet::Commands::Peripheral::FindInformation>()
    .register_command<Packet::Commands::Peripheral::FindByType>()
    .register_command<Packet::Errors::ErrorResponse>()
    .register_event<Packet::Events::DeviceConnected>()
    .register_event<Packet::Events::DeviceDisconnected>()
    .register_event<Packet::Events::AdvertisingReport>()
    .register_event<Packet::Events::CommandComplete>()
    .register_event<Packet::Events::CommandStatus>()
    .register_event<Packet::Events::NumberOfCompletedPackets>()
    .set_ignored_packets({
      Format::HCI::SubEventCode::LEReadRemoteUsedFeaturesComplete,
      Format::HCI::SubEventCode::LEConnectionUpdateComplete,
      Format::HCI::CommandCode::ConnectionParameterUpdateRequest
    });
}

// Stdio
void register_stdio_packets(PacketBuilder& stdio_packet_builder) {
  stdio_packet_builder
    .register_command<Packet::Commands::GetControllersIdsRequest>()
    .register_command<Packet::Commands::GetControllerInfoRequest>()
    .register_command<Packet::Commands::GetConnectedDevices>()
    .register_command<Packet::Commands::SetPoweredRequest>()
    .register_command<Packet::Commands::SetDiscoverableRequest>()
    .register_command<Packet::Commands::SetConnectableRequest>()
    .register_command<Packet::Commands::CreateConnection>()
    .register_command<Packet::Commands::CancelConnectionRequest>()
    .register_command<Packet::Commands::Disconnect>()
    .register_command<Packet::Commands::Central::ReadRequest>()
    .register_command<Packet::Commands::Peripheral::ReadResponse>()
    .register_command<Packet::Commands::Central::WriteRequest>()
    .register_command<Packet::Commands::Peripheral::WriteResponse>()
    .register_command<Packet::Commands::Central::WriteWithoutResponse>()
    .register_command<Packet::Commands::Peripheral::EmitNotification>()
    .register_command<Packet::Meta::GetControllersList>()
    .register_command<Packet::Meta::SetAdvertising>()
    .register_command<Packet::Meta::StartScan>()
    .register_command<Packet::Meta::StopScan>()
    .register_command<Packet::Meta::ProbeServices>()
    .register_command<Packet::Meta::ProbeCharacteristics>()
    .register_command<Packet::Control::SetGATTTable>()
    .register_command<Packet::Control::Exit>();
}

#endif //BABLE_REGISTRATION_CPP
