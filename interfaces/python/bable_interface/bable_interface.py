import logging
import os
import subprocess
import threading
import time
from .BaBLE import Payload, Exit
from .threads import WorkingThread, ReceivingThread
from .commands import CommandsManager
from .utils import none_cb
from .models import Packet


class BaBLEInterface(object):

    def __init__(self):
        self.logger = logging.getLogger(__name__)

        self.working_thread = None
        self.working_ready_event = threading.Event()

        self.receiving_thread = None

        self.subprocess = None
        self.subprocess_ready_event = threading.Event()

        self.commands_manager = None
        self.started = False

        self.on_error = None

    def start(self, on_error=None, use_path=False, exit_on_sigint=True):
        if self.started:
            raise RuntimeError("BaBLEInterface already running")

        logging_lvl = logging.getLevelName(self.logger.getEffectiveLevel())

        if use_path:
            bable_bridge_path = 'bable-bridge-linux'
        else:
            bable_bridge_path = os.path.join(os.path.dirname(__file__), 'bin', 'bable-bridge-linux')

        command = [bable_bridge_path, '--logging', logging_lvl]
        if not exit_on_sigint:
            command.append('--disable-signal')

        self.subprocess = subprocess.Popen(
            command,
            stdout=subprocess.PIPE, stdin=subprocess.PIPE,
            bufsize=0,
            universal_newlines=False
        )

        self.on_error = on_error

        self.working_ready_event.clear()
        self.subprocess_ready_event.clear()

        self.working_thread = WorkingThread(self.working_ready_event)
        self.working_thread.setDaemon(True)
        self.working_thread.start()
        self.working_ready_event.wait()

        self.commands_manager = CommandsManager(self.subprocess)

        self.receiving_thread = ReceivingThread(self._on_receive, self.subprocess.stdout)
        self.receiving_thread.setDaemon(True)
        self.receiving_thread.start()

        self.subprocess_ready_event.wait()
        self.started = True

    def stop(self, sync=True):
        if not self.started:
            raise RuntimeError("BaBLEInterface not running")
        self.started = False

        self.working_thread.stop(sync)

        self.commands_manager.send_packet(Packet.build(Exit))

        while self.subprocess.poll() is None:
            self.logger.info("Waiting for subprocess to stop")
            time.sleep(0.1)

        self.receiving_thread.join(5)
        if self.receiving_thread.isAlive():
            self.logger.warning("Timeout while waiting for receiving thread to stop...")

    def _on_receive(self, packet):
        if packet.payload_type == Payload.Payload.Ready:
            self.subprocess_ready_event.set()
            return

        if packet.payload_type == Payload.Payload.BaBLEError:
            if not packet.packet_uuid.uuid and self.on_error is not None:
                self.working_thread.add_task(task=self.commands_manager.handle_error(packet, self.on_error))
                return

        self.commands_manager.handle(packet, self.working_thread.add_task)

    def _run_command(self, command_name, params, sync):
        if not self.started:
            raise RuntimeError("BaBLEInterface not running")

        def on_done(fut, event, return_value):
            try:
                return_value.update({
                    'success': True,
                    'result': fut.result(),
                    'failure_reason': None
                })
            except Exception as err:
                return_value.update({
                    'success': False,
                    'result': None,
                    'failure_reason': err
                })

            event.set()

        done_event = threading.Event()
        result = {}

        command = getattr(self.commands_manager, command_name)
        self.working_thread.add_task(task=command(*params), callback=on_done, event=done_event, return_value=result)

        if sync:
            done_event.wait()
            if result.get('success', False):
                return result.get('result')
            else:
                raise result.get('failure_reason', RuntimeError("Command failed without given failure reason"))

        return None

    #### Commands ####
    def start_scan(self, on_device_found, active_scan=True, controller_id=0, on_scan_started=none_cb, sync=True,
                   timeout=15.0):
        return self._run_command(
            command_name='start_scan',
            params=[controller_id, active_scan, on_device_found, on_scan_started, timeout],
            sync=sync
        )

    def stop_scan(self, controller_id=0, on_scan_stopped=none_cb, sync=True, timeout=15.0):
        return self._run_command(
            command_name='stop_scan',
            params=[controller_id, on_scan_stopped, timeout],
            sync=sync
        )

    def connect(self, address, address_type, on_connected=none_cb, on_disconnected=none_cb, controller_id=0, sync=False,
                timeout=15.0):
        return self._run_command(
            command_name='connect',
            params=[controller_id, address, address_type, on_connected, on_disconnected, timeout],
            sync=sync
        )

    def disconnect(self, connection_handle, on_disconnected=none_cb, controller_id=0, sync=False, timeout=15.0):
        return self._run_command(
            command_name='disconnect',
            params=[controller_id, connection_handle, on_disconnected, timeout],
            sync=sync
        )

    def cancel_connection(self, controller_id=0, on_connection_cancelled=none_cb, sync=False, timeout=15.0):
        return self._run_command(
            command_name='cancel_connection',
            params=[controller_id, on_connection_cancelled, timeout],
            sync=sync
        )

    def list_connected_devices(self, controller_id=0, timeout=15.0):
        return self._run_command(
            command_name='list_connected_devices',
            params=[controller_id, timeout],
            sync=True
        )

    def list_controllers(self, timeout=15.0):
        return self._run_command(
            command_name='list_controllers',
            params=[timeout],
            sync=True
        )

    def read(self, connection_handle, attribute_handle, on_read=none_cb, controller_id=0, sync=False, timeout=15.0):
        return self._run_command(
            command_name='read',
            params=[controller_id, connection_handle, attribute_handle, on_read, timeout],
            sync=sync
        )

    def write(self, connection_handle, attribute_handle, value, on_written=none_cb, controller_id=0, sync=False,
              timeout=15.0):
        return self._run_command(
            command_name='write',
            params=[controller_id, connection_handle, attribute_handle, value, on_written, timeout],
            sync=sync
        )

    def write_without_response(self, connection_handle, attribute_handle, value, controller_id=0, sync=False):
        return self._run_command(
            command_name='write_without_response',
            params=[controller_id, connection_handle, attribute_handle, value],
            sync=sync
        )

    def set_notification(self, enabled, connection_handle, characteristic, on_notification_set=none_cb,
                         on_notification_received=none_cb, controller_id=0, sync=True, timeout=15.0):
        return self._run_command(
            command_name='set_notification',
            params=[controller_id, enabled, connection_handle, characteristic, on_notification_set,
                    on_notification_received, timeout],
            sync=sync
        )

    def set_gatt_table(self, services, characteristics, on_set=none_cb, controller_id=0, sync=True, timeout=15.0):
        return self._run_command(
            command_name='set_gatt_table',
            params=[controller_id, services, characteristics, on_set, timeout],
            sync=sync
        )

    def set_advertising(self, enabled, uuids=None, name=None, company_id=None, advertising_data=None,
                        scan_response=None, on_set=none_cb, controller_id=0, sync=True, timeout=15.0):
        return self._run_command(
            command_name='set_advertising',
            params=[controller_id, enabled, uuids, name, company_id, advertising_data, scan_response, on_set, timeout],
            sync=sync
        )

    def notify(self, connection_handle, attribute_handle, value, controller_id=0, sync=False):
        return self._run_command(
            command_name='notify',
            params=[controller_id, connection_handle, attribute_handle, value],
            sync=sync
        )

    #### Handlers registration ####
    def on_write_request(self, on_write_handler):
        return self._run_command(
            command_name='on_write_request',
            params=[on_write_handler],
            sync=True
        )

    def on_read_request(self, on_read_handler):
        return self._run_command(
            command_name='on_read_request',
            params=[on_read_handler],
            sync=True
        )
