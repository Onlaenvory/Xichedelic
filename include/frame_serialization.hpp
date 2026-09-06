#pragma once

#include "spdlog/spdlog.h"
#include <cstddef>
#include <cstdint>

namespace XI {
  enum Opcode : uint8_t {
    Continuation_f = 0,
    Text_f         = 1,
    Binary_f       = 2,
    Close_f        = 8,
    Ping_f         = 9,
    Pong_f         = 10
  };

  struct Frame{
    uint8_t byte[14] = {0};
    uint8_t headerSize = 0;

    void set_FIN(bool fin) { byte[0] = (byte[0] & ~0x80) | (fin << 7); }
    void set_RSV1(bool rsv1) { byte[0] = (byte[0] & ~0x40) | (rsv1 << 6); }
    void set_RSV2(bool rsv2) { byte[0] = (byte[0] & ~0x20) | (rsv2 << 5); }
    void set_RSV3(bool rsv3) { byte[0] = (byte[0] & ~0x10) | (rsv3 << 4); }
    void set_OPCODE(Opcode type) { byte[0] = (byte[0] & 0xF0) | (type & 0x0F); }

    void set_MASK(bool mask) { byte[1] = (byte[1] & ~0x80) | (mask << 7); }
    void set_PAYLOAD_SIZE(size_t length) {
      byte[1] &= 0x80;

      if (length <= 125) {
        byte[1] |= (length & 127);
        headerSize = 2;
      }
      else if (length <= 65535) {
        byte[1] |= 126;
        byte[2] = (length >> 8) & 255;
        byte[3] = length & 255;
        headerSize = 4;
      }
      else {
        byte[1] |= 127;
        byte[2] = (length >> 56) & 255;
        byte[3] = (length >> 48) & 255;
        byte[4] = (length >> 40) & 255;
        byte[5] = (length >> 32) & 255;
        byte[6] = (length >> 24) & 255;
        byte[7] = (length >> 16) & 255;
        byte[8] = (length >> 8) & 255;
        byte[9] = length & 255;
        headerSize = 10;
      };

      bool isMasked = (byte[1] & 0x80) != 0;
      if (isMasked) { headerSize += 4; }
    }

    void set_MASK_KEY(uint32_t key) {
      if (headerSize < 6) return;

      uint8_t keyPos = headerSize - 4;
      byte[keyPos] = (key >> 24) & 0xFF;
      byte[keyPos + 1] = (key >> 16) & 0xFF;
      byte[keyPos + 2] = (key >> 8) & 0xFF;
      byte[keyPos + 3] = key & 0xFF;
    };

    uint8_t* get_MASK_KEY() {
      if (headerSize < 6) return nullptr;
      return &byte[headerSize - 4];
    }
  };

  inline void maskPayload(uint8_t* payload, size_t length, uint8_t maskKey[4]) {
    for (size_t i = 0; i < length; i++) {
      payload[i] ^= maskKey[i % 4];
    }
    spdlog::info("Mask Payload Successful");
  };
}
