import asyncio
from asyncio import Future
import uuid

from .BaBLE import Payload, DeviceFound, StartScan, StopScan, ProbeServices, ProbeCharacteristics, DeviceConnected, \
    Connect, Disconnect, CancelConnection, DeviceDisconnected, GetConnectedDevices, GetControllersList, Read, Write, \
    WriteWithoutResponse
from .flatbuffer import extract_packet


@asyncio.coroutine
def start_scan(self, controller_id, on_device_found):

    @asyncio.coroutine
    def on_device_found_event(packet):
        result = extract_packet(
            packet=packet,
            payload_class=DeviceFound.DeviceFound,
            params=["type", "address", "address_type", "rssi", "uuid", "company_id", "device_name"],
            numpy_params=["manufacturer_data"]
        )

        on_device_found(True, result, None)

    @asyncio.coroutine
    def on_start_scan_response(packet, future):
        print("ON START SCAN RESPONSE", packet.Status())
        future.set_result(None)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_event_callback(
        payload_type=Payload.Payload.DeviceFound, controller_id=controller_id, callback_fn=on_device_found_event
    )
    self.register_response_callback(uuid=uuid_request, callback_fn=on_start_scan_response, future=future)

    self.send_packet(
        payload_module=StartScan,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"active_scan": True}
    )

    print("Waiting for scan to start...")
    yield from asyncio.wait_for(future, 15.0)


@asyncio.coroutine
def stop_scan(self, controller_id):

    @asyncio.coroutine
    def on_stop_scan_response(packet, future):
        print("ON STOP SCAN RESPONSE", packet.Status())
        self.remove_event_callback(payload_type=Payload.Payload.DeviceFound, controller_id=packet.ControllerId())
        future.set_result(None)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_stop_scan_response, future=future)

    self.send_packet(payload_module=StopScan, uuid=uuid_request, controller_id=controller_id)

    print("Waiting for scan to stop...")
    yield from asyncio.wait_for(future, 15.0)  # TODO: add timeout parameter


@asyncio.coroutine
def probe_services(self, controller_id, connection_handle):

    @asyncio.coroutine
    def on_services_probed(packet, future):
        print("ON PROBE SERVICES RESPONSE", packet.Status())
        result = extract_packet(
            packet=packet,
            payload_class=ProbeServices.ProbeServices,
            list_params=["services"]
        )

        services = []
        for service in result["services"]:
            services.append({
                "handle": service.Handle(),
                "group_end_handle": service.GroupEndHandle(),
                "uuid": service.Uuid()
            })

        future.set_result(services)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_services_probed, future=future)

    self.send_packet(
        payload_module=ProbeServices,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"connection_handle": connection_handle}
    )

    print("Waiting for services...")
    services = yield from asyncio.wait_for(future, 15.0)

    return services


@asyncio.coroutine
def probe_characteristics(self, controller_id, connection_handle):

    @asyncio.coroutine
    def on_characteristics_probed(packet, future):
        print("ON PROBE CHARACTERISTICS RESPONSE", packet.Status())
        result = extract_packet(
            packet=packet,
            payload_class=ProbeCharacteristics.ProbeCharacteristics,
            list_params=["characteristics"]
        )

        characteristics = []
        for characteristic in result["characteristics"]:
            characteristics.append({
                "handle": characteristic.Handle(),
                "value_handle": characteristic.ValueHandle(),
                "config_handle": characteristic.ConfigHandle(),
                "notification_enabled": characteristic.NotificationEnabled(),
                "indication_enabled": characteristic.IndicationEnabled(),
                "uuid": characteristic.Uuid(),
                "indicate": characteristic.Indicate(),
                "notify": characteristic.Notify(),
                "read": characteristic.Read(),
                "write": characteristic.Write(),
                "broadcast": characteristic.Broadcast()
            })

        future.set_result(characteristics)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_characteristics_probed, future=future)

    self.send_packet(
        payload_module=ProbeCharacteristics,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"connection_handle": connection_handle}
    )

    print("Waiting for characteristics...")
    characteristics = yield from asyncio.wait_for(future, 15.0)

    return characteristics


@asyncio.coroutine
def connect(self, controller_id, address, address_type, on_connected_with_info, on_disconnected):

    @asyncio.coroutine
    def on_unexpected_disconnection(packet):
        print("ON UNEXPECTED DISCONNECTION", packet.Status())
        data = extract_packet(
            packet=packet,
            payload_class=DeviceDisconnected.DeviceDisconnected,
            params=["connection_handle", "reason"]
        )
        data["controller_id"] = packet.ControllerId()
        self.remove_event_callback(
            payload_type=Payload.Payload.DeviceDisconnected,
            controller_id=data["controller_id"],
            connection_handle=data["connection_handle"]
        )

        on_disconnected(True, data, None)

    @asyncio.coroutine
    def on_connected(packet, future):
        print("ON DEVICE CONNECTED", packet.Status())
        self.remove_event_callback(payload_type=Payload.Payload.DeviceConnected, controller_id=packet.ControllerId())
        device = extract_packet(
            packet=packet,
            payload_class=DeviceConnected.DeviceConnected,
            params=["connection_handle", "address", "address_type"]
        )
        device["controller_id"] = packet.ControllerId()

        self.register_event_callback(
            payload_type=Payload.Payload.DeviceDisconnected,
            controller_id=controller_id,
            connection_handle=device["connection_handle"],
            callback_fn=on_unexpected_disconnection
        )
        future.set_result(None)

        device["services"] = yield from self.probe_services(device["controller_id"], device["connection_handle"])
        device["characteristics"] = yield from self.probe_characteristics(
            device["controller_id"],
            device["connection_handle"]
        )

        on_connected_with_info(True, device, None)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_event_callback(
        payload_type=Payload.Payload.DeviceConnected,
        controller_id=controller_id,
        address=address,
        callback_fn=on_connected,
        future=future
    )

    self.send_packet(
        payload_module=Connect,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"address": address, "address_type": 0 if address_type == "public" else 1}
    )

    print("Connecting...")

    # TODO: add timeout connection
    try:
        yield from asyncio.wait_for(future, 5.0)
    except asyncio.TimeoutError:
        print("CONNECTION TIMEOUT")
        self.remove_event_callback(payload_type=Payload.Payload.DeviceConnected, controller_id=controller_id)
        on_connected_with_info(False, None, "Connection timed out")


@asyncio.coroutine
def disconnect(self, controller_id, connection_handle, on_disconnected):

    @asyncio.coroutine
    def on_device_disconnected(packet, future):
        print("ON DEVICE DISCONNECTED", packet.Status())
        data = extract_packet(
            packet=packet,
            payload_class=DeviceDisconnected.DeviceDisconnected,
            params=["connection_handle", "reason"]
        )
        data["controller_id"] = packet.ControllerId()
        self.remove_event_callback(
            payload_type=Payload.Payload.DeviceDisconnected,
            controller_id=data["controller_id"],
            connection_handle=data["connection_handle"]
        )

        future.set_result(None)

        on_disconnected(True, data, None)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_event_callback(
        replace=True,
        payload_type=Payload.Payload.DeviceDisconnected,
        controller_id=controller_id,
        connection_handle=connection_handle,
        callback_fn=on_device_disconnected,
        future=future
    )

    self.send_packet(
        payload_module=Disconnect,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"connection_handle": connection_handle}
    )

    print("Disconnecting...")
    try:
        yield from asyncio.wait_for(future, 5.0)
    except asyncio.TimeoutError:
        print("DISCONNECTION TIMEOUT")
        self.remove_event_callback(
            payload_type=Payload.Payload.DeviceDisconnected,
            controller_id=controller_id,
            connection_handle=connection_handle
        )
        on_disconnected(False, None, "Disconnection timed out")


@asyncio.coroutine
def cancel_connection(self, controller_id):

    @asyncio.coroutine
    def on_connection_cancelled(packet, future):
        print("ON CONNECTION CANCELLED RESPONSE", packet.Status())
        self.remove_event_callback(payload_type=Payload.Payload.CancelConnection, controller_id=packet.ControllerId())

        future.set_result(None)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_connection_cancelled, future=future)

    self.send_packet(payload_module=CancelConnection, uuid=uuid_request, controller_id=controller_id)

    print("Waiting for connection to cancel...")
    yield from asyncio.wait_for(future, 15.0)


@asyncio.coroutine
def list_connected_devices(self, controller_id):

    @asyncio.coroutine
    def on_response_received(packet, future):
        print("ON LIST CONNECTED DEVICES RESPONSE", packet.Status())

        result = extract_packet(
            packet=packet,
            payload_class=GetConnectedDevices.GetConnectedDevices,
            list_params=["devices"]
        )

        future.set_result(result["devices"])

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_response_received, future=future)

    self.send_packet(payload_module=GetConnectedDevices, uuid=uuid_request, controller_id=controller_id)

    print("Waiting for list of connected devices...")
    result = yield from asyncio.wait_for(future, 15.0)

    return result


@asyncio.coroutine
def list_controllers(self):

    @asyncio.coroutine
    def on_response_received(packet, future):
        print("ON LIST CONTROLLERS RESPONSE", packet.Status())

        result = extract_packet(
            packet=packet,
            payload_class=GetControllersList.GetControllersList,
            list_params=["controllers"]
        )
        controllers = []
        for controller in result["controllers"]:
            controllers.append({
                "id": controller.Id(),
                "address": controller.Address(),
                "name": controller.Name(),
                "powered": controller.Powered(),
                "connectable": controller.Connectable(),
                "discoverable": controller.Discoverable(),
                "low_energy": controller.LowEnergy()
            })

        future.set_result(controllers)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_response_received, future=future)

    self.send_packet(payload_module=GetControllersList, uuid=uuid_request)

    print("Waiting for list of controllers...")
    result = yield from asyncio.wait_for(future, 15.0)

    return result


@asyncio.coroutine
def read(self, controller_id, connection_handle, attribute_handle, on_read):

    @asyncio.coroutine
    def on_response_received(packet, future):
        print("ON READ RESPONSE", packet.Status())
        data = extract_packet(
            packet=packet,
            payload_class=Read.Read,
            params=["connection_handle", "attribute_handle"],
            numpy_params=["value"]
        )
        data["controller_id"] = packet.ControllerId()

        future.set_result(None)

        on_read(True, data, None)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_response_received, future=future)

    self.send_packet(
        payload_module=Read,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"connection_handle": connection_handle, "attribute_handle": attribute_handle}
    )

    print("Reading...")
    try:
        yield from asyncio.wait_for(future, 5.0)
    except asyncio.TimeoutError:
        print("READ TIMEOUT")
        self.remove_response_callback(uuid=uuid_request)
        on_read(False, None, "Read timed out")


@asyncio.coroutine
def write(self, controller_id, connection_handle, attribute_handle, value, on_written):

    @asyncio.coroutine
    def on_response_received(packet, future):
        print("ON WRITE RESPONSE", packet.Status())
        data = extract_packet(
            packet=packet,
            payload_class=Write.Write,
            params=["connection_handle", "attribute_handle"]
        )
        data["controller_id"] = packet.ControllerId()

        future.set_result(None)

        on_written(True, data, None)

    future = Future()
    uuid_request = str(uuid.uuid4())

    self.register_response_callback(uuid=uuid_request, callback_fn=on_response_received, future=future)

    self.send_packet(
        payload_module=Write,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"connection_handle": connection_handle, "attribute_handle": attribute_handle, "value": value}
    )

    print("Writing...")
    try:
        yield from asyncio.wait_for(future, 5.0)
    except asyncio.TimeoutError:
        print("WRITE TIMEOUT")
        self.remove_response_callback(uuid=uuid_request)
        on_written(False, None, "Write timed out")


@asyncio.coroutine
def write_without_response(self, controller_id, connection_handle, attribute_handle, value):
    uuid_request = str(uuid.uuid4())

    self.send_packet(
        payload_module=WriteWithoutResponse,
        uuid=uuid_request,
        controller_id=controller_id,
        params={"connection_handle": connection_handle, "attribute_handle": attribute_handle, "value": value}
    )

    print("Write without response command sent")
