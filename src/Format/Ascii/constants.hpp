#ifndef BABLE_LINUX_ASCII_CONSTANTS_HPP
#define BABLE_LINUX_ASCII_CONSTANTS_HPP

namespace Format::Ascii {

  const char Delimiter = ',';

  enum CommandCode {
    GetMGMTInfo= 0x0001,
    StartScan= 0x0002,
    StopScan= 0x0003,
    AddDevice= 0x0004,
    RemoveDevice= 0x0005,
    Disconnect= 0x0006,
    SetPowered= 0x0007,
    SetDiscoverable= 0x0008,
    SetConnectable= 0x0009,
    GetControllersList= 0x000A,
  };

  enum EventCode {
    DeviceFound= 0x0100,
    Discovering= 0x0200,
    DeviceConnected= 0x0300,
    DeviceDisconnected=0x0400,
    ClassOfDeviceChanged= 0x0500,
    NewSettings= 0x0600
  };

}

#endif //BABLE_LINUX_ASCII_CONSTANTS_HPP
