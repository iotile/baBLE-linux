#include <uv.h>
#include "Log/Log.hpp"
#include "Application/PacketBuilder/PacketBuilder.hpp"
#include "Application/PacketRouter/PacketRouter.hpp"
#include "Format/Flatbuffers/FlatbuffersFormat.hpp"
#include "Transport/Socket/MGMT/MGMTSocket.hpp"
#include "Transport/Socket/HCI/HCISocket.hpp"
#include "Transport/Socket/StdIO/StdIOSocket.hpp"
#include "Transport/SocketContainer/SocketContainer.hpp"
#include "Exceptions/AbstractException.hpp"
#include "Exceptions/RuntimeError/RuntimeErrorException.hpp"
#include "Application/Packets/Errors/BaBLEError/BaBLEError.hpp"
#include "Application/Packets/Control/Ready/Ready.hpp"
#include "bootstrap.hpp"

#define EXPIRATION_DURATION_MS 60 * 1000

using namespace std;

// Function used to call all handlers closing callbacks before stopping the loop
void cleanly_stop_loop(uv_loop_t* loop) {
  uv_walk(loop, [](uv_handle_t* handle, void* arg){
    uv_close(handle, [](uv_handle_t* handle) {});
  }, nullptr);

  uv_stop(loop);
  LOG.debug("Handles stopped.");
}

void parse_options(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    string option_name = string(argv[i]);
    if (option_name == "--logging") {
      if (i + 1 < argc) {
        LOG.set_level(string(argv[++i]));
      } else {
        throw invalid_argument("Invalid option:"
                               "--logging option requires one argument [DEBUG|INFO|WARNING|ERROR|CRITICAL|NOTSET]");
      }
    } else {
      fprintf(stderr, "Unknown option given (%s)\n", option_name.c_str());
    }
  }
}

int main(int argc, char* argv[]) {
  parse_options(argc, argv);

  // Create loop
  uv_loop_t* loop = uv_default_loop();
  uv_loop_init(loop);
  LOG.debug("Loop created.");

  // Stop on SIGINT signal
  uv_signal_t stop_signal;
  uv_signal_init(loop, &stop_signal);
  uv_signal_start(&stop_signal, [](uv_signal_t* handle, int signum) {
    LOG.warning("Stop signal received...", "Signal");
    cleanly_stop_loop(handle->loop);
  }, SIGINT);
  LOG.debug("SIGINT signal handled.");

  // Formats
  LOG.info("Creating formats...");
  shared_ptr<HCIFormat> hci_format = make_shared<HCIFormat>();
  shared_ptr<MGMTFormat> mgmt_format = make_shared<MGMTFormat>();
  shared_ptr<FlatbuffersFormat> fb_format = make_shared<FlatbuffersFormat>();

  // Sockets
  LOG.info("Creating sockets...");
  shared_ptr<MGMTSocket> mgmt_socket = make_shared<MGMTSocket>(loop, mgmt_format);
  shared_ptr<StdIOSocket> stdio_socket = make_shared<StdIOSocket>(loop, fb_format);
  vector<shared_ptr<HCISocket>> hci_sockets = HCISocket::create_all(loop, hci_format);

  // Socket container
  LOG.info("Registering sockets into socket container...");
  SocketContainer socket_container{};
  socket_container
      .register_socket(mgmt_socket)
      .register_socket(stdio_socket);
  for (auto& hci_socket : hci_sockets) {
    socket_container.register_socket(hci_socket);
  }

  // PacketRouter
  shared_ptr<PacketRouter> packet_router = make_shared<PacketRouter>(loop);

  // PacketBuilder
  LOG.info("Registering packets into packet builder...");
  PacketBuilder mgmt_packet_builder(mgmt_format);
  Bootstrap::register_mgmt_packets(mgmt_packet_builder);

  PacketBuilder hci_packet_builder(hci_format);
  Bootstrap::register_hci_packets(hci_packet_builder);

  PacketBuilder stdio_packet_builder(fb_format);
  Bootstrap::register_stdio_packets(stdio_packet_builder);

  // Poll sockets
  LOG.info("Creating socket pollers...");

  auto on_error = [&stdio_socket, &socket_container](const Exceptions::AbstractException& err) {
    LOG.error(err.stringify(), "Error");
    shared_ptr<Packet::Errors::BaBLEError> error_packet = make_shared<Packet::Errors::BaBLEError>(err);
    socket_container.send(error_packet);
  };

  mgmt_socket->poll(
    [&mgmt_packet_builder, &packet_router, &socket_container](const vector<uint8_t>& received_data, const shared_ptr<AbstractFormat>& format) {
      shared_ptr<AbstractExtractor> extractor = format->create_extractor(received_data);

      shared_ptr<Packet::AbstractPacket> packet = mgmt_packet_builder.build(extractor);
      if (packet == nullptr) {
        return;
      }
      LOG.debug("Packet built", "MGMT poller");

      packet = packet_router->route(packet_router, packet);
      LOG.debug("Packet routed", "MGMT poller");

      packet->prepare(packet_router);
      LOG.debug("Packet prepared to be sent", "MGMT poller");

      socket_container.send(packet);
      LOG.debug("Packet sent", "MGMT poller");
    },
    on_error
  );

  for (auto& hci_socket : hci_sockets) {
    hci_socket->poll(
      [&hci_socket, &hci_packet_builder, &packet_router, &socket_container](const vector<uint8_t>& received_data, const shared_ptr<AbstractFormat>& format) {
        // Create extractor from raw bytes received
        shared_ptr<AbstractExtractor> extractor = format->create_extractor(received_data);
        extractor->set_controller_id(hci_socket->get_controller_id());

        // Build packet
        shared_ptr<Packet::AbstractPacket> packet = hci_packet_builder.build(extractor);
        if (packet == nullptr) {
          return;
        }
        LOG.debug("Packet built", "HCI poller");

        // This part is needed due to a bug since Linux Kernel v4 : we have to create manually the L2CAP socket, else
        // we'll be disconnected after sending one packet.
        if (packet->get_id() == Packet::Id::DeviceConnected) {
          if (packet->get_status() != BaBLE::StatusCode::Success) {
            LOG.debug("Unsuccessful DeviceConnected event ignored (because the device is not connected...)", "HCI poller");
            return;
          }

          auto device_connected_packet = dynamic_pointer_cast<Packet::Events::DeviceConnected>(packet);
          if (device_connected_packet == nullptr) {
            throw Exceptions::RuntimeErrorException("Can't downcast packet to DeviceConnected packet");
          }

          LOG.debug("DeviceConnected", "HCI poller");
          hci_socket->connect_l2cap_socket(
              device_connected_packet->get_connection_handle(),
              device_connected_packet->get_raw_device_address(),
              device_connected_packet->get_device_address_type()
          );
        } else if (packet->get_id() == Packet::Id::DeviceDisconnected) {
          if (packet->get_status() != BaBLE::StatusCode::Success) {
            LOG.debug("Unsuccessful DeviceDisconnected event ignored (because the device is not disconnected...)", "HCI poller");
            return;
          }

          auto device_disconnected_packet = dynamic_pointer_cast<Packet::Events::DeviceDisconnected>(packet);
          if (device_disconnected_packet == nullptr) {
            throw Exceptions::RuntimeErrorException("Can't downcast packet to DeviceDisconnected packet");
          }

          LOG.debug("DeviceDisconnected", "HCI poller");
          hci_socket->disconnect_l2cap_socket(device_disconnected_packet->get_connection_handle());
        }

        // Check if there are packets waiting for a response
        packet = packet_router->route(packet_router, packet);
        LOG.debug("Packet routed", "HCI poller");

        packet->prepare(packet_router);
        LOG.debug("Packet prepared to be sent", "HCI poller");

        socket_container.send(packet);
        LOG.debug("Packet sent", "HCI poller");
      },
      on_error
    );
  }

  stdio_socket->poll(
    [&stdio_packet_builder, &packet_router, &socket_container, &loop](const vector<uint8_t>& received_data, const shared_ptr<AbstractFormat>& format) {
      shared_ptr<AbstractExtractor> extractor = format->create_extractor(received_data);

      shared_ptr<Packet::AbstractPacket> packet = stdio_packet_builder.build(extractor);
      if (packet == nullptr) {
        return;
      }
      LOG.debug("Packet built", "BABLE poller");

      packet->prepare(packet_router);
      LOG.debug("Packet prepared to be sent", "BABLE poller");

      socket_container.send(packet);
      LOG.debug("Packet sent", "BABLE poller");

      if (packet->get_id() == Packet::Id::Exit) {
        LOG.debug("Received Exit packet. Cleanly stopping loop...");
        cleanly_stop_loop(loop);
      }
    },
    on_error
  );
  stdio_socket->on_close([&loop]() {
    cleanly_stop_loop(loop);
  });

  // Removed all expired waiting response
  LOG.info("Creating expiration timer...");
  packet_router->start_expiration_timer(EXPIRATION_DURATION_MS);

  // Send Ready packet to indicate that BaBLE has started and is ready to receive commands
  LOG.info("Sending Ready packet...");
  shared_ptr<Packet::Control::Ready> ready_packet = make_shared<Packet::Control::Ready>();
  socket_container.send(ready_packet);

  // Start the loop
  LOG.info("Starting loop...");

  uv_run(loop, UV_RUN_DEFAULT);
  LOG.info("Loop stopped.");

  // Close and clean the loop
  int result = uv_loop_close(loop);
  if (result != 0) {
    LOG.critical("Failed to close libuv loop: " + string(uv_err_name(result)));
  }

  return 0;
}
