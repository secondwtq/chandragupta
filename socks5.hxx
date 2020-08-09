#ifndef _HEADER_SOCKS5_
#define _HEADER_SOCKS5_

#include <cstddef>
#include <optional>
#include <string>
#include <array>
#include <sstream>
#include <cassert>
#include <cstdint>

namespace socks5 {

const constexpr std::byte VER = static_cast<std::byte>(0x05);
const constexpr std::byte FIELD_RSV = static_cast<std::byte>(0x00);

const constexpr std::byte CMD_CONNECT = static_cast<std::byte>(0x01);
const constexpr std::byte CMD_BIND = static_cast<std::byte>(0x02);
const constexpr std::byte CMD_UDP_ASSOCIATE = static_cast<std::byte>(0x03);

const constexpr std::byte ATYP_IPV4 = static_cast<std::byte>(0x01);
const constexpr std::byte ATYP_DOMAIN = static_cast<std::byte>(0x03);
const constexpr std::byte ATYP_IPV6 = static_cast<std::byte>(0x04);

enum CommandType {
  CmdConnect,
  CmdBind,
  CmdUDPAssociate,
};

enum AddressType {
  AddrIPv4,
  AddrDomain,
  AddrIPv6,
};

struct Address {
private:
  const std::array<std::byte, 4> ipv4;
  const std::string domain;
public:
  const AddressType kind;
  Address(const std::array<std::byte, 4> &ipv4) : kind(AddrIPv4), ipv4(ipv4), domain() {}
  Address(const std::string &domain) : kind(AddrDomain), ipv4(), domain(domain) {}

  const std::string& getDomain() const {
    assert(kind == AddrDomain);
    return domain;
  }

  const std::array<std::byte, 4>& getIPv4() const {
    assert(kind == AddrIPv4);
    return ipv4;
  }

  std::string dump() const {
    switch(kind) {
      case AddrIPv4: {
        std::stringstream ss;
        ss << static_cast<uint16_t>(ipv4[0]) << "."
           << static_cast<uint16_t>(ipv4[1]) << "."
           << static_cast<uint16_t>(ipv4[2]) << "."
           << static_cast<uint16_t>(ipv4[3]);
        return ss.str();
      }
      case AddrDomain:
        return domain;
      case AddrIPv6:
        assert(false);
    }
    // TODO: make an unreachable
    return "";
  }
};

struct Command {
  const CommandType kind;
  const Address address;
  const uint16_t port;
  Command(CommandType kind, Address address, uint16_t port)
      : kind(kind), address(address), port(port) {}
};

static std::optional<CommandType> byteToCommandType(std::byte src) {
  switch (src) {
  case CMD_CONNECT:
    return CmdConnect;
  case CMD_BIND:
    return CmdBind;
  case CMD_UDP_ASSOCIATE:
    return CmdUDPAssociate;
  default:
    return {};
  }
}

static std::optional<AddressType> byteToAddressType(std::byte src) {
  switch (src) {
  case ATYP_IPV4:
    return AddrIPv4;
  case ATYP_DOMAIN:
    return AddrDomain;
  case ATYP_IPV6:
    return AddrIPv6;
  default:
    return {};
  }
}

}

#endif
