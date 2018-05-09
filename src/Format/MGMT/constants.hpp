#ifndef BABLE_LINUX_MGMT_CONSTANTS_HPP
#define BABLE_LINUX_MGMT_CONSTANTS_HPP

#include <cstdint>

namespace Format::MGMT {

  const std::size_t header_length = 6;
  const uint16_t non_controller_id = 0xFFFF;

  enum CommandCode {
    GetMGMTInfo= 0x0001,
    StartScan= 0x0023,
    StopScan= 0x0024
  };

  enum EventCode {
    DeviceFound= 0x0012,
    Discovering= 0x0013
  };

  enum StatusCode {
    Success= 0x00,
    UnknownCommand= 0x01,
    NotConnected= 0x02,
    Failed= 0x03,
    ConnectFailed= 0x04,
    AuthenticationFailed= 0x05,
    NotPaired= 0x06,
    NoResources= 0x07,
    Timeout= 0x08,
    AlreadyConnected= 0x09,
    Busy= 0x0A,
    Rejected= 0x0B,
    NotSupported= 0x0C,
    InvalidParameters= 0x0D,
    Disconnected= 0x0E,
    NotPowered= 0x0F,
    Cancelled= 0x10,
    InvalidIndex= 0x11,
    RFKilled= 0x12,
    AlreadyPaired= 0x13,
    PermissionDenied= 0x14
  };

}

#endif //BABLE_LINUX_MGMT_CONSTANTS_HPP