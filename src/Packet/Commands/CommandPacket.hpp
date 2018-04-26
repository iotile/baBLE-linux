#ifndef BABLE_LINUX_COMMANDPACKET_HPP
#define BABLE_LINUX_COMMANDPACKET_HPP

#include "../AbstractPacket.hpp"
#include "../../Serializer/Serializer.hpp"
#include "../constants.hpp"

namespace Packet::Commands {

  template<class T>
  class CommandPacket : public AbstractPacket {

  protected:
    CommandPacket(Packet::Type initial_type, Packet::Type translated_type): AbstractPacket(initial_type, translated_type) {
      m_command_code = T::command_code(m_current_type);
      m_controller_id = 0xFFFF;
      m_params_length = 0;
    };

    std::string to_ascii() const override {
      std::stringstream result;
      generate_header(result);

      return result.str();
    };

    Serializer to_mgmt() const override {
      Serializer ser;
      generate_header(ser);

      return ser;
    };

    void after_translate() override {
      m_command_code = T::command_code(m_current_type);
    };

    uint16_t m_command_code;
    uint16_t m_controller_id;
    uint16_t m_params_length;

  private:
    void generate_header(Serializer& ser) const {
      switch(m_current_type) {
        case Packet::MGMT:
          ser << m_command_code << m_controller_id << m_params_length;
          break;

        default:
          throw std::runtime_error("Can't generate header for current packet type.");
      }
    }

    void generate_header(std::stringstream& str) const {
      switch(m_current_type) {
        case Packet::ASCII:
          str << "<CommandPacket> "
              << "Controller ID: " << std::to_string(m_controller_id) << ", "
              << "Parameters length: " << std::to_string(m_params_length) << ", "
              << "Command code: " << std::to_string(m_command_code);
          break;

        default:
          throw std::runtime_error("Can't generate header for current packet type.");
      }
    }

  };

}

#endif //BABLE_LINUX_COMMANDPACKET_HPP
