#pragma once

namespace FTL {
class System {
  public:
    ~System() = default;
    virtual bool init() = 0;
    virtual bool shutdown() = 0;
};
}; // namespace FTL
