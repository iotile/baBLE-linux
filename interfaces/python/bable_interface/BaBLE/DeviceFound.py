# automatically generated by the FlatBuffers compiler, do not modify

# namespace: BaBLE

import flatbuffers

class DeviceFound(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsDeviceFound(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = DeviceFound()
        x.Init(buf, n + offset)
        return x

    # DeviceFound
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DeviceFound
    def Type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # DeviceFound
    def Address(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # DeviceFound
    def AddressType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # DeviceFound
    def Rssi(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int8Flags, o + self._tab.Pos)
        return 0

    # DeviceFound
    def Uuid(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # DeviceFound
    def CompanyId(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint16Flags, o + self._tab.Pos)
        return 0

    # DeviceFound
    def ManufacturerData(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    # DeviceFound
    def ManufacturerDataAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Uint8Flags, o)
        return 0

    # DeviceFound
    def ManufacturerDataLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # DeviceFound
    def DeviceName(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def DeviceFoundStart(builder): builder.StartObject(8)
def DeviceFoundAddType(builder, type): builder.PrependUint8Slot(0, type, 0)
def DeviceFoundAddAddress(builder, address): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(address), 0)
def DeviceFoundAddAddressType(builder, addressType): builder.PrependUint8Slot(2, addressType, 0)
def DeviceFoundAddRssi(builder, rssi): builder.PrependInt8Slot(3, rssi, 0)
def DeviceFoundAddUuid(builder, uuid): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(uuid), 0)
def DeviceFoundAddCompanyId(builder, companyId): builder.PrependUint16Slot(5, companyId, 0)
def DeviceFoundAddManufacturerData(builder, manufacturerData): builder.PrependUOffsetTRelativeSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(manufacturerData), 0)
def DeviceFoundStartManufacturerDataVector(builder, numElems): return builder.StartVector(1, numElems, 1)
def DeviceFoundAddDeviceName(builder, deviceName): builder.PrependUOffsetTRelativeSlot(7, flatbuffers.number_types.UOffsetTFlags.py_type(deviceName), 0)
def DeviceFoundEnd(builder): return builder.EndObject()
