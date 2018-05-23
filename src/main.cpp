#include <memory>
#include <uvw.hpp>
#include "Log/Log.hpp"
#include "Format/Ascii/AsciiFormat.hpp"
#include "Format/MGMT/MGMTFormat.hpp"
#include "Format/Flatbuffers/FlatbuffersFormat.hpp"
#include "Transport/Socket/MGMT/MGMTSocket.hpp"
#include "Transport/Socket/HCI/HCISocket.hpp"
#include "Transport/Socket/StdIO/StdIOSocket.hpp"
#include "Transport/SocketContainer/SocketContainer.hpp"
#include "Application/PacketContainer/PacketContainer.hpp"
#include "Exceptions/AbstractException.hpp"
#include "Application/Packets/Errors/BaBLEError/BaBLEErrorPacket.hpp"
#include "bootstrap.hpp"
#include "Application/Packets/Control/Ready/Ready.hpp"
#include "Exceptions/RuntimeError/RuntimeErrorException.hpp"

#define EXPIRATION_DURATION_SECONDS 15

using namespace std;
using namespace uvw;

// Function used to call all handlers closing callbacks before stopping the loop
void cleanly_stop_loop(Loop& loop) {
  loop.walk([](BaseHandle &handle){
    handle.close();
  });
  loop.stop();
  LOG.debug("Handles stopped.");
}

int main() {
  ENABLE_LOGGING(DEBUG);

  // Create loop
  shared_ptr<Loop> loop = Loop::getDefault();
  loop->on<ErrorEvent>([] (const ErrorEvent& err, Loop& l) {
    LOG.error("An error occured: " + string(err.what()), "Loop");
  });
  LOG.debug("Loop created.");

  // Stop on SIGINT signal
  shared_ptr<SignalHandle> stop_signal = loop->resource<SignalHandle>();
  stop_signal->on<SignalEvent>([] (const SignalEvent&, SignalHandle& sig) {
    LOG.warning("Stop signal received...", "Signal");
    cleanly_stop_loop(sig.loop());
  });
  stop_signal->start(SIGINT);
  LOG.debug("SIGINT signal handled.");

  // Formats
  LOG.info("Creating formats...");
  shared_ptr<HCIFormat> hci_format = make_shared<HCIFormat>();
  shared_ptr<MGMTFormat> mgmt_format = make_shared<MGMTFormat>();
  shared_ptr<FlatbuffersFormat> fb_format = make_shared<FlatbuffersFormat>();
  shared_ptr<AsciiFormat> ascii_format = make_shared<AsciiFormat>();

  // Sockets
  LOG.info("Creating sockets...");
  shared_ptr<MGMTSocket> mgmt_socket = make_shared<MGMTSocket>(mgmt_format);
  shared_ptr<StdIOSocket> stdio_socket = make_shared<StdIOSocket>(fb_format);
  vector<shared_ptr<HCISocket>> hci_sockets = Bootstrap::create_hci_sockets(mgmt_socket, hci_format);

  // Socket container
  LOG.info("Registering sockets into socket container...");
  SocketContainer socket_container;
  socket_container
      .register_socket(mgmt_socket)
      .register_socket(stdio_socket);
  for (auto& hci_socket : hci_sockets) {
    socket_container.register_socket(hci_socket);
  }

  // PacketContainer
  LOG.info("Registering packets into packet container...");
  PacketContainer mgmt_packet_container(mgmt_format);
  Bootstrap::register_mgmt_packets(mgmt_packet_container, stdio_socket->format());

  PacketContainer hci_packet_container(hci_format);
  Bootstrap::register_hci_packets(hci_packet_container, stdio_socket->format());

  PacketContainer stdio_packet_container(stdio_socket->format());
  Bootstrap::register_stdio_packets(stdio_packet_container, mgmt_format, hci_format);

  // Poll sockets
  LOG.info("Creating socket pollers...");

  auto on_error = [&stdio_socket, &socket_container](const Exceptions::AbstractException& err) {
    LOG.error(err.stringify(), "Error");
    std::shared_ptr<Packet::Errors::BaBLEErrorPacket> error_packet = make_shared<Packet::Errors::BaBLEErrorPacket>(
        stdio_socket->format()->get_packet_type()
    );
    error_packet->from_exception(err);
    socket_container.send(error_packet);
  };

  mgmt_socket->poll(
    loop,
    [&mgmt_packet_container, &socket_container](const std::vector<uint8_t>& received_data, const std::shared_ptr<AbstractFormat>& format) {
      shared_ptr<AbstractExtractor> extractor = format->create_extractor(received_data);
      std::shared_ptr<Packet::AbstractPacket> packet = mgmt_packet_container.build(extractor);
      LOG.debug("Packet built", "MGMT poller");

      packet->translate();
      LOG.debug("Packet translated", "MGMT poller");

      PacketContainer::register_response(packet);

      socket_container.send(packet);
    },
    on_error
  );

  for (auto& hci_socket : hci_sockets) {
    hci_socket->poll(
      loop,
      [&hci_packet_container, &socket_container, &hci_socket](const std::vector<uint8_t>& received_data, const std::shared_ptr<AbstractFormat>& format) {
          shared_ptr<AbstractExtractor> extractor = format->create_extractor(received_data);
          extractor->set_controller_id(hci_socket->get_controller_id());
          std::shared_ptr<Packet::AbstractPacket> packet = hci_packet_container.build(extractor);
          LOG.debug("Packet built", "HCI poller");

          // This part is needed due to a bug since Linux Kernel v4 : we have to create manually the L2CAP socket, else
          // we'll be disconnected after sending one packet.
          if (packet->get_id() == BaBLE::Payload::DeviceConnected) {
            auto device_connected_packet = std::dynamic_pointer_cast<Packet::Events::DeviceConnected>(packet);
            if (device_connected_packet == nullptr) {
              throw Exceptions::RuntimeErrorException("Can't downcast packet to DeviceConnected packet");
            }

            hci_socket->connect_l2cap_socket(
                device_connected_packet->get_connection_handle(),
                device_connected_packet->get_raw_device_address(),
                device_connected_packet->get_device_address_type()
            );
          } else if (packet->get_id() == BaBLE::Payload::DeviceDisconnected) {
            auto device_disconnected_packet = std::dynamic_pointer_cast<Packet::Events::DeviceDisconnected>(packet);
            if (device_disconnected_packet == nullptr) {
              throw Exceptions::RuntimeErrorException("Can't downcast packet to DeviceDisconnected packet");
            }
            hci_socket->disconnect_l2cap_socket(device_disconnected_packet->get_connection_handle());
          }

          packet->translate();
          LOG.debug("Packet translated", "HCI poller");

          PacketContainer::register_response(packet);

          socket_container.send(packet);
      },
      on_error
    );
  }

  // TODO: comment and clean includes
  stdio_socket->poll(
    loop,
    [&stdio_packet_container, &socket_container, &loop](const std::vector<uint8_t>& received_data, const std::shared_ptr<AbstractFormat>& format) {
      shared_ptr<AbstractExtractor> extractor = format->create_extractor(received_data);
      std::shared_ptr<Packet::AbstractPacket> packet = stdio_packet_container.build(extractor);

      LOG.debug("Packet built", "BABLE poller");

      if (packet->get_id() == BaBLE::Payload::Exit) {
        LOG.debug("Received Exit packet. Cleanly stopping loop...");
        cleanly_stop_loop(*loop);
        return;
      }

      packet->translate();
      LOG.debug("Packet translated", "BABLE poller");

      PacketContainer::register_response(packet);

      socket_container.send(packet);
    },
    on_error
  );

  LOG.info("Creating expiration timer...");
  shared_ptr<TimerHandle> expiration_timer = loop->resource<TimerHandle>();
  expiration_timer->on<TimerEvent>([](const TimerEvent&, TimerHandle& t) {
    PacketContainer::expire_waiting_packets(EXPIRATION_DURATION_SECONDS);
  });
  expiration_timer->start(chrono::seconds(EXPIRATION_DURATION_SECONDS), chrono::seconds(EXPIRATION_DURATION_SECONDS));

  // Send Ready packet to indicate that BaBLE has started and is ready to receive commands
  LOG.info("Sending Ready packet...");
  std::shared_ptr<Packet::Control::Ready> ready_packet = make_shared<Packet::Control::Ready>(stdio_socket->format()->get_packet_type());
  socket_container.send(ready_packet);

  // Start the loop
  LOG.info("Starting loop...");
  loop->run<Loop::Mode::DEFAULT>();
  LOG.info("Loop stopped.");

  // Close and clean the loop
  loop->close();
  loop->clear();
  return 0;
}
