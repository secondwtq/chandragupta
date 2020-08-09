#ifndef _HEADER_CLIENT_
#define _HEADER_CLIENT_

#include "socks5.hxx"
#include "Socks5Impl.hxx"
#include <spdlog/spdlog.h>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

class MauryaClientContext {
private:
  asio::io_context asioContext;
public:
  MauryaClientContext() { }
  void run() {
    asioContext.run();
  }

  asio::io_context& getASIOContext() {
    return asioContext;
  }
};

enum SOCKSClientConnectionState {
  CLIENT_INIT,
  CLIENT_AUTHED,
  CLIENT_ERROR,
};

class SOCKSClientConnection
    : public std::enable_shared_from_this<SOCKSClientConnection> {
private:
  MauryaClientContext &ctx;
  asio::ip::tcp::socket asioTCPSocket;
  SOCKSClientConnectionState state = CLIENT_INIT;
public:
  SOCKSClientConnection(MauryaClientContext &ctx)
      : ctx(ctx), asioTCPSocket(ctx.getASIOContext()) {}
  asio::ip::tcp::socket& getASIOTCPSocket() {
    return asioTCPSocket;
  }

  void run() {
    this->readClientIdent();
  }

  void reportError(const std::string& message) {

  }

  void readClientIdent() {
    assert(state == CLIENT_INIT);
    socks5_impl::readClientIdent(
        asioTCPSocket, [this](const asio::error_code &err) {
          socks5_impl::readClientCommand(
              asioTCPSocket, [this](const asio::error_code &err,
                                    const std::optional<socks5::Command> &maybeCmd) {
                if (err)
                  return spdlog::error(err.message());
                auto cmd = maybeCmd.value();
                spdlog::info("client ident read {} {} {}", cmd.kind, cmd.address.dump(),
                             cmd.port);
              });
        });
  }
};

class SOCKSClientListener {
private:
  MauryaClientContext &ctx;
  std::vector<std::shared_ptr<SOCKSClientConnection>> connections;
  asio::ip::tcp::acceptor asioAcceptor;

public:
  SOCKSClientListener(MauryaClientContext &ctx)
      : ctx(ctx),
        asioAcceptor(ctx.getASIOContext(),
                     asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2030)) {}

  void startListen() {
    std::shared_ptr<SOCKSClientConnection> conn =
        std::make_shared<SOCKSClientConnection>(ctx);
    asioAcceptor.async_accept(conn->getASIOTCPSocket(),
                              [this, conn](const asio::error_code &err) {
                                if (err) {
                                  spdlog::error(err.message());
                                } else {
                                  connections.push_back(conn);
                                  conn->run();
                                }
                                this->startListen();
                              });
  }
};

#endif
