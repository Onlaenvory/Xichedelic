#pragma once

#include <cstdint>
#include <vector>

namespace XI {
  enum Opcode : uint8_t {
    Continuation = 0x0,
    Text         = 0x1,
    Binary       = 0x2,
    Close        = 0x8,
    Ping         = 0x9,
    Pong         = 0xA
  };

  struct Frame {
    bool fin = true;
    Opcode opcode = Opcode::Text;
    bool masked = false;
    uint32_t mask_key = 0;
    std::vector<uint8_t> payload;
  };




  class FrameEncoder {

  };
}
