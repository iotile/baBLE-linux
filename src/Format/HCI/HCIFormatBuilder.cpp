#include "HCIFormatBuilder.hpp"

using namespace std;

// Constructors
HCIFormatBuilder::HCIFormatBuilder(uint16_t controller_id) {
  m_opcode = 0;
  m_controller_id = controller_id;
  m_connection_handle = 0;
};

// Setters
HCIFormatBuilder& HCIFormatBuilder::set_opcode(uint8_t code) {
  m_opcode = code;
  return *this;
}

HCIFormatBuilder& HCIFormatBuilder::set_controller_id(uint16_t controller_id) {
  m_controller_id = controller_id;
  return *this;
}

HCIFormatBuilder& HCIFormatBuilder::set_connection_handle(uint16_t connection_handle) {
  m_connection_handle = connection_handle;
  return *this;
}

// To add data to current building object
HCIFormatBuilder& HCIFormatBuilder::add(const string& value) {
  for (const char& c : value) {
    this->add<char>(c);
  }
  this->add<char>('\0');

  return *this;
};

// To build and finish the result object
vector<uint8_t> HCIFormatBuilder::build() {
  vector<uint8_t> result = generate_header();

  result.insert(result.end(), m_formatted_data.begin(), m_formatted_data.end());

  return result;
};

// Private
vector<uint8_t> HCIFormatBuilder::generate_header() {
  auto data_length = static_cast<uint16_t>(m_formatted_data.size());
  uint16_t l2cap_length = sizeof(m_opcode) + data_length;
  auto total_length = static_cast<uint16_t>(l2cap_length + 4);

  vector<uint8_t> header;
  header.reserve(Format::HCI::async_data_header_length); // TODO: fix -> need arg to build() to adapt header

  uint16_t connection_handle_with_flags = m_connection_handle | Format::HCI::HandleFlag::StartNonFlush << 12;

  // Use little-endian
  header.push_back(Format::HCI::AsyncData);
  header.push_back(static_cast<uint8_t>(connection_handle_with_flags & 0x00FF));
  header.push_back(static_cast<uint8_t>(connection_handle_with_flags >> 8));

  header.push_back(static_cast<uint8_t>(total_length & 0x00FF));
  header.push_back(static_cast<uint8_t>(total_length >> 8));

  header.push_back(static_cast<uint8_t>(l2cap_length & 0x00FF));
  header.push_back(static_cast<uint8_t>(l2cap_length >> 8));

  header.push_back(static_cast<uint8_t>(ATT_CID & 0x00FF));
  header.push_back(static_cast<uint8_t>(ATT_CID >> 8));

  header.push_back(m_opcode);

  return header;
};
