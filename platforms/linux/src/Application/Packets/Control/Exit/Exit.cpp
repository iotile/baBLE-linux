#include <sstream>
#include "Exit.hpp"

using namespace std;

namespace Packet {

  namespace Control {

    Exit::Exit()
        : HostOnlyPacket(Packet::Id::Exit, initial_packet_code()) {}

    void Exit::unserialize(FlatbuffersFormatExtractor& extractor) {}

    vector<uint8_t> Exit::serialize(FlatbuffersFormatBuilder& builder) const {
      auto payload = BaBLE::CreateExit(builder);

      return builder.build(payload, BaBLE::Payload::Exit);
    }

    void Exit::set_socket(AbstractSocket* socket) {
      socket->close();
    }

    const string Exit::stringify() const {
      stringstream result;

      result << "<Exit> "
             << AbstractPacket::stringify();

      return result.str();
    }

  }

}