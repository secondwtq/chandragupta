
#include "Client.hxx"
#include "Socks5Impl.hxx"

MauryaClientContext::MauryaClientContext(GlobalContext &globalContext)
    : globalContext(globalContext),
      proxyListener(std::make_unique<SOCKSClientListener>(*this)) {}

void MauryaClientContext::addClientConnection(
    std::unique_ptr<SOCKSClientConnection> conn) {
  SOCKSClientConnection *connPtr = conn.get();
  clientConnections.emplace(std::move(conn));
  connPtr->run();
}

void SOCKSClientListener::startListen() {
  std::unique_ptr<SOCKSClientConnection> conn =
      std::make_unique<SOCKSClientConnection>(ctx);

  asio::ip::tcp::socket& socket = conn->getASIOTCPSocket();
  asioAcceptor.async_accept(socket, [this, conn = std::move(conn)](
                                        const asio::error_code &err) mutable {
    if (err)
      spdlog::error(err.message());
    else
      ctx.addClientConnection(std::move(conn));
    this->startListen();
  });
}

void SOCKSClientConnection::run() {
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
