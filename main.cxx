
#include "GlobalContext.hxx"
#include "Client.hxx"
#include <spdlog/spdlog.h>
#include <cxxopts.hpp>
#include <simdjson.h>
#include <memory>
#include <string>
#include <string_view>
#include <cstdint>

struct Endpoint {
  const std::string address;
  const uint16_t port;
  Endpoint(std::string_view address, uint16_t port)
      : address(address), port(port) {}
};

class DecreeBase {
public:
  virtual ~DecreeBase() {}
  virtual void run(GlobalContext &globalContext) = 0;
};

class DecreeLaunchClient : public DecreeBase {
private:
  Endpoint server, local;

public:
  DecreeLaunchClient(Endpoint server, Endpoint local)
      : server(server), local(local) {}
  virtual void run(GlobalContext &globalContext) {
    std::unique_ptr<MauryaClientContext> clientContext =
        std::make_unique<MauryaClientContext>(globalContext);
    MauryaClientContext *clientContextPtr = clientContext.get();
    globalContext.addGlobalObject(std::move(clientContext));
    clientContextPtr->getProxyListener()->startListen();
  }
};

class DecreeLaunchServer : public DecreeBase {

};

std::unique_ptr<DecreeBase> readDecree(simdjson::dom::object src) {
  std::string_view decreeKind = src["decree"];
  if (decreeKind == "launchClient") {
    simdjson::dom::object specifics = src["specifics"];
    Endpoint server{
        specifics["serverAddress"],
        static_cast<uint16_t>(static_cast<uint64_t>(specifics["serverPort"]))};
    Endpoint local{
        specifics["localAddress"],
        static_cast<uint16_t>(static_cast<uint64_t>(specifics["localPort"]))};
    std::unique_ptr<DecreeLaunchClient> ret = std::make_unique<DecreeLaunchClient>(server, local);
    return ret;
  } else if (decreeKind == "launchServer") {
    spdlog::error("Unimplemented");
  } else {
    spdlog::error("Invalid decree {}", decreeKind);
  }
  return nullptr;
}

int main(int argc, char *argv[]) {
  spdlog::info("From the first stirrings of life beneath water... to the great \
beasts of the Stone Age... to man taking his first upright steps, you have \
come far. Now begins your greatest quest: from this early cradle of \
civilization on towards the stars.");

  try {
    GlobalContext globalContext;

    std::string config = "";
    cxxopts::Options options(argv[0]);
    options.add_options()("c,config", "Config File",
                          cxxopts::value<std::string>(config));

    auto result = options.parse(argc, argv);

    if (config.empty()) {
      spdlog::error("Need config file");
      return 0;
    }

    simdjson::dom::parser jsonParser;
    simdjson::dom::element jsonConfig = jsonParser.load(config);
    simdjson::dom::array jsonArray = jsonConfig["decrees"];

    std::vector<std::unique_ptr<DecreeBase>> decrees;
    for (simdjson::dom::object decreeDesc : jsonArray) {
      auto decree = readDecree(decreeDesc);
      if (!decree)
        return 0;
      decrees.push_back(std::move(decree));
    }
    for (auto& decree : decrees)
      decree->run(globalContext);

    globalContext.getGlobalASIOContext().run();
  } catch (std::exception& e) {
    spdlog::error(e.what());
  }

  return 0;
}
