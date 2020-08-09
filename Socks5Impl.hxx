#ifndef _HEADER_SOCKS5_IMPL_
#define _HEADER_SOCKS5_IMPL_

#include "socks5.hxx"
#include <algorithm>
#include <array>
#include <asio/bind_executor.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <cstddef>
#include <memory>
#include <vector>

namespace socks5_impl {

enum ErrorCode {
  ERR_VERSION_MISMATCH,
  ERR_MISC,
  ERR_UNSUPPORTED_OPERATION,
};

class ErrorCategory : public asio::error_category {
public:
  const char *name() const noexcept { return "socks5"; }

  std::string message(int value) const {
    switch (value) {
    case ERR_VERSION_MISMATCH:
      return "Version mismatch";
    case ERR_UNSUPPORTED_OPERATION:
      return "Unsupported Operation";
    case ERR_MISC:
    default:
      return "SOCKS5 misc error";
    }
  }
};

extern ErrorCategory errorCategory;

template <class CompletionToken>
auto readClientIdent(asio::ip::tcp::socket &socket, CompletionToken &&token) {
  auto initiation = [&socket](auto &&completionHandler) {
    auto callback = [&socket, completionHandler](const asio::error_code &err) {
      auto executor = asio::get_associated_executor(completionHandler,
                                                    socket.get_executor());
      return asio::post(
          asio::bind_executor(executor, std::bind(completionHandler, err)));
    };

    std::shared_ptr<std::array<std::byte, 2>> clientIdentBuf =
        std::make_shared<std::array<std::byte, 2>>();
    asio::async_read(
        socket, asio::buffer(*clientIdentBuf),
        [&socket, clientIdentBuf, callback](const asio::error_code &err,
                                            size_t byteTransferred) {
          if (err)
            return callback(err);
          if ((*clientIdentBuf)[0] != socks5::VER)
            return callback({ERR_VERSION_MISMATCH, errorCategory});
          size_t nMethods = static_cast<size_t>((*clientIdentBuf)[1]);
          if (nMethods < 1)
            return callback({ERR_MISC, errorCategory});

          std::shared_ptr<std::vector<std::byte>> authMethodBuf =
              std::make_shared<std::vector<std::byte>>(nMethods);
          asio::async_read(
              socket, asio::buffer(*authMethodBuf),
              [&socket, authMethodBuf, callback](const asio::error_code &err,
                                                 size_t byteTransferred) {
                if (err)
                  return callback(err);
                if (std::find(authMethodBuf->begin(), authMethodBuf->end(),
                              static_cast<std::byte>(0x00)) ==
                    authMethodBuf->end())
                  return callback({ERR_UNSUPPORTED_OPERATION, errorCategory});

                std::shared_ptr<std::array<std::byte, 2>> authRespBuf =
                    std::make_shared<std::array<std::byte, 2>>();
                (*authRespBuf) = {socks5::VER, static_cast<std::byte>(0x00)};
                asio::async_write(
                    socket, asio::buffer(*authRespBuf),
                    [&socket, callback](const asio::error_code &err,
                                        size_t bytesTransferred) {
                      return callback(err);
                    });
              });
        });
  };

  return asio::async_initiate<CompletionToken, void(const asio::error_code &)>(
      initiation, token);
}

template <class CompletionToken>
auto readClientAddress(asio::ip::tcp::socket &socket,
                       socks5::AddressType addrType, CompletionToken &&token) {
  auto initiation = [&socket, addrType](auto &&completionHandler) {
    auto callback = [&socket, completionHandler](
                        const asio::error_code &err,
                        const std::optional<socks5::Address> &addr) {
      auto executor = asio::get_associated_executor(completionHandler,
                                                    socket.get_executor());
      return asio::post(asio::bind_executor(
          executor, std::bind(completionHandler, err, addr)));
    };

    switch (addrType) {
    case socks5::AddrIPv4: {
      std::shared_ptr<std::array<std::byte, 4>> addrBuf =
          std::make_shared<std::array<std::byte, 4>>();
      asio::async_read(socket, asio::buffer(*addrBuf),
                       [addrBuf, callback](const asio::error_code &err,
                                           size_t byteTransferred) {
                         if (err)
                           return callback(err, {});
                         callback(err, socks5::Address(*addrBuf));
                       });
      break;
    }
    case socks5::AddrDomain: {
      std::shared_ptr<uint8_t> lengthBuf = std::make_shared<uint8_t>();
      asio::async_read(
          socket, asio::buffer(lengthBuf.get(), 1),
          [&socket, lengthBuf, callback](const asio::error_code &err,
                                         size_t byteTransferred) {
            if (err)
              return callback(err, {});
            std::shared_ptr<std::vector<std::byte>> domainBuf =
                std::make_shared<std::vector<std::byte>>(*lengthBuf);
            asio::async_read(
                socket, asio::buffer(*domainBuf),
                [domainBuf, callback](const asio::error_code &err,
                                      size_t byteTransferred) {
                  if (err)
                    return callback(err, {});
                  std::string domain(
                      reinterpret_cast<const char *>(&((*domainBuf)[0])),
                      domainBuf->size());
                  callback(err, socks5::Address(domain));
                });
          });
      break;
    }
    case socks5::AddrIPv6:
      return callback({ERR_UNSUPPORTED_OPERATION, errorCategory}, {});
    }
  };

  return asio::async_initiate<CompletionToken,
                              void(const asio::error_code &err,
                                   const std::optional<socks5::Address> &cmd)>(
      initiation, token);
}

template <class CompletionToken>
auto readClientCommand(asio::ip::tcp::socket &socket, CompletionToken &&token) {
  auto initiation = [&socket](auto &&completionHandler) {
    auto callback = [&socket, completionHandler](
                        const asio::error_code &err,
                        const std::optional<socks5::Command> &cmd) {
      auto executor = asio::get_associated_executor(completionHandler,
                                                    socket.get_executor());
      return asio::post(asio::bind_executor(
          executor, std::bind(completionHandler, err, cmd)));
    };

    std::shared_ptr<std::array<std::byte, 4>> headerBuf =
        std::make_shared<std::array<std::byte, 4>>();
    asio::async_read(
        socket, asio::buffer(*headerBuf),
        [&socket, headerBuf, callback](const asio::error_code &err,
                                       size_t byteTransferred) {
          if (err)
            return callback(err, {});
          if ((*headerBuf)[0] != socks5::VER)
            return callback({ERR_VERSION_MISMATCH, errorCategory}, {});
          if ((*headerBuf)[2] != socks5::FIELD_RSV)
            return callback({ERR_MISC, errorCategory}, {});

          std::optional<socks5::CommandType> maybeCommandType =
              socks5::byteToCommandType((*headerBuf)[1]);
          if (!maybeCommandType)
            return callback({ERR_MISC, errorCategory}, {});
          socks5::CommandType commandType = maybeCommandType.value();

          std::optional<socks5::AddressType> maybeAddressType =
              socks5::byteToAddressType((*headerBuf)[3]);
          if (!maybeAddressType)
            return callback({ERR_MISC, errorCategory}, {});
          socks5::AddressType addressType = maybeAddressType.value();

          readClientAddress(
              socket, addressType,
              [&socket, callback,
               commandType](const asio::error_code &err,
                            const std::optional<socks5::Address> &addr) {
                if (err)
                  return callback(err, {});
                std::shared_ptr<std::array<std::byte, 2>> portBuf =
                    std::make_shared<std::array<std::byte, 2>>();
                asio::async_read(
                    socket, asio::buffer(*portBuf),
                    [portBuf, callback, commandType, addr](
                        const asio::error_code &err, size_t byteTransferred) {
                      if (err)
                        return callback(err, {});
                      // TODO: Byte order
                      std::swap((*portBuf)[0], (*portBuf)[1]);
                      callback(err,
                               socks5::Command(commandType, addr.value(),
                                               *reinterpret_cast<uint16_t *>(
                                                   &(*portBuf)[0])));
                    });
              });
        });
  };

  return asio::async_initiate<CompletionToken,
                              void(const asio::error_code &err,
                                   const std::optional<socks5::Command> &cmd)>(
      initiation, token);
}

} // namespace socks5_impl

#endif
