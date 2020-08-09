#ifndef _HEADER_CLIENT_
#define _HEADER_CLIENT_

#include "GlobalContext.hxx"
#include <spdlog/spdlog.h>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>
#include <set>

class MauryaClientContext;
class SOCKSClientListener;
class SOCKSClientConnection;

class MauryaClientContext : public GlobalObject {
private:
  GlobalContext &globalContext;
  std::unique_ptr<SOCKSClientListener> proxyListener;
  std::set<std::unique_ptr<SOCKSClientConnection>> clientConnections;

public:
  MauryaClientContext(GlobalContext &globalContext);
  void addClientConnection(std::unique_ptr<SOCKSClientConnection> conn);

  SOCKSClientListener *getProxyListener() {
    return proxyListener.get();
  }

  asio::io_context& getASIOContext() {
    return globalContext.getGlobalASIOContext();
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

  void run();

  void reportError(const std::string& message) {

  }
};

class SOCKSClientListener {
private:
  MauryaClientContext &ctx;
  asio::ip::tcp::acceptor asioAcceptor;

public:
  SOCKSClientListener(MauryaClientContext &ctx)
      : ctx(ctx),
        asioAcceptor(ctx.getASIOContext(),
                     asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2030)) {}

  void startListen();

};

#endif
