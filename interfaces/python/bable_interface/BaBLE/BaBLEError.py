# automatically generated by the FlatBuffers compiler, do not modify

# namespace: BaBLE

import flatbuffers

class BaBLEError(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsBaBLEError(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = BaBLEError()
        x.Init(buf, n + offset)
        return x

    # BaBLEError
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # BaBLEError
    def Message(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def BaBLEErrorStart(builder): builder.StartObject(1)
def BaBLEErrorAddMessage(builder, message): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(message), 0)
def BaBLEErrorEnd(builder): return builder.EndObject()
