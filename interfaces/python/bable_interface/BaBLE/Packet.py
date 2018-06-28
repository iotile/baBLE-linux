# automatically generated by the FlatBuffers compiler, do not modify

# namespace: BaBLE

import flatbuffers

class Packet(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsPacket(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Packet()
        x.Init(buf, n + offset)
        return x

    # Packet
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Packet
    def Uuid(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Packet
    def PayloadType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Packet
    def Payload(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

    # Packet
    def ControllerId(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint16Flags, o + self._tab.Pos)
        return 65535

    # Packet
    def Status(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Packet
    def NativeStatus(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Packet
    def NativeClass(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def PacketStart(builder): builder.StartObject(7)
def PacketAddUuid(builder, uuid): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(uuid), 0)
def PacketAddPayloadType(builder, payloadType): builder.PrependUint8Slot(1, payloadType, 0)
def PacketAddPayload(builder, payload): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(payload), 0)
def PacketAddControllerId(builder, controllerId): builder.PrependUint16Slot(3, controllerId, 65535)
def PacketAddStatus(builder, status): builder.PrependUint8Slot(4, status, 0)
def PacketAddNativeStatus(builder, nativeStatus): builder.PrependUint8Slot(5, nativeStatus, 0)
def PacketAddNativeClass(builder, nativeClass): builder.PrependUOffsetTRelativeSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(nativeClass), 0)
def PacketEnd(builder): return builder.EndObject()
