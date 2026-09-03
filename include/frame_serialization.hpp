#pragma once

#include <cstdint>
#include <vector>

namespace XI {
  // rfc6455 #section-5 defined opcode
  enum opcode : uint8_t {
    Continuation_f = 0, // %x0
    Text_f         = 1, // %x1
    Binary_f       = 2, // %x2
    Close_f        = 8, // %x8
    Ping_f         = 9, // %x9
    Pong_f         = 10 // %xA
  }; // opcode.md

  // FIN, RSV (1-3) Expect to return 0xF (1111)
  struct HeaderFlags {
    bool fin  = true;
    bool rsv1 = false;
    bool rsv2 = false;
    bool rsv3 = false;

    uint8_t serialize() {
      uint8_t byte = 0x0;
      if (fin) byte |= (1 << 7);
      if (rsv1) byte |= (1 << 6);
      if (rsv2) byte |= (1 << 5);
      if (rsv3) byte |= (1 << 4);
      return byte;
    }

    static HeaderFlags deserialize(uint8_t byte_0) {
      HeaderFlags flags;
      flags.fin = (byte_0 >> 7) & 0x01;
      flags.rsv1 = (byte_0 >> 6) & 0x01;
      flags.rsv2 = (byte_0 >> 5) & 0x01;
      flags.rsv3 = (byte_0 >> 4) & 0x01;
      return flags;
    }
}
