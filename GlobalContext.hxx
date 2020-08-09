#ifndef _HEADER_GLOBAL_CONTEXT_
#define _HEADER_GLOBAL_CONTEXT_

#include <asio/io_context.hpp>
#include <set>
#include <memory>

class GlobalObject {
public:
  virtual ~GlobalObject() { }
};

class GlobalContext {
private:
  std::set<std::unique_ptr<GlobalObject>> globalObjects;
  asio::io_context globalASIOContext;
public:

  GlobalContext() { }

  void addGlobalObject(std::unique_ptr<GlobalObject> src) {
    globalObjects.emplace(std::move(src));
  }

  asio::io_context &getGlobalASIOContext() { return globalASIOContext; }

private:
};

#endif
