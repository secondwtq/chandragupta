
#include "Client.hxx"
#include <spdlog/spdlog.h>
#include <memory>
#include <array>
#include <string>

int main() {
  spdlog::info("From the first stirrings of life beneath water... to the great \
beasts of the Stone Age... to man taking his first upright steps, you have \
come far. Now begins your greatest quest: from this early cradle of \
civilization on towards the stars.");

  try {
    MauryaClientContext ctx;
    SOCKSClientListener listener(ctx);
    listener.startListen();
    ctx.run();
  } catch (std::exception& e) {
    spdlog::error(e.what());
  }

  return 0;
}
