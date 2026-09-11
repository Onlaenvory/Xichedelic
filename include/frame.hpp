#pragma once

#include <spdlog/spdlog.h>
#include <opcode.hpp>
#include <cstddef>
#include <cstdint>

namespace XI {
struct HeaderFrame {
  uint8_t byte[14] = {0};
  uint8_t headerSize = 0;

  void SetFin(bool fin) {
    byte[0] = (byte[0] & ~0x80) | (static_cast<uint8_t>(fin) << 7);
  }
  void SetRsv1(bool rsv1) {
    byte[0] = (byte[0] & ~0x40) | (static_cast<uint8_t>(rsv1) << 6);
  }
  void SetRsv2(bool rsv2) {
    byte[0] = (byte[0] & ~0x20) | (static_cast<uint8_t>(rsv2) << 5);
  }
  void SetRsv3(bool rsv3) {
    byte[0] = (byte[0] & ~0x10) | (static_cast<uint8_t>(rsv3) << 4);
  }
  void SetOpcode(Opcode opcode) {
    byte[0] = (byte[0] & 0xF0) | (static_cast<uint8_t>(opcode) & 0x0F);
  }

  void SetMask(bool mask) {
    byte[1] = (byte[1] & ~0x80) | (static_cast<uint8_t>(mask) << 7);
  }
  void SetPayloadSize(size_t size) {
    byte[1] &= 0x80;

    if (size <= 125) {
      byte[1] |= (size & 0x7F);
      headerSize = 2;
    }
    else if (size <= 65535) {
      byte[1] |= 0x7E;
      byte[2] = (size >> 8) & 0xFF;
      byte[3] = size & 0xFF;
      headerSize = 4;
    }
    else {
      byte[1] |= 0x7F;
      byte[2] = (size >> 56) & 0xFF;
      byte[3] = (size >> 48) & 0xFF;
      byte[4] = (size >> 40) & 0xFF;
      byte[5] = (size >> 32) & 0xFF;
      byte[6] = (size >> 24) & 0xFF;
      byte[7] = (size >> 16) & 0xFF;
      byte[8] = (size >> 8) & 0xFF;
      byte[9] = size & 0xFF;
      headerSize = 10;
    }

    if ((byte[1] & 0x80) != 0) {
      headerSize += 4;
    }
  }

  void SetMaskKey(uint32_t key) {
    if (headerSize < 6) {
      return;
    }
    uint8_t keyByte = headerSize - 4;

    byte[keyByte] = (key >> 24) & 0xFF;
    byte[keyByte + 1] = (key >> 16) & 0xFF;
    byte[keyByte + 2] = (key >> 8) & 0xFF;
    byte[keyByte + 3] = key & 0xFF;
  };
  uint8_t *MaskKey() {
    if (headerSize < 6) {
      return nullptr;
    }
    return &byte[headerSize - 4];
  }
};

inline void MaskPayload(uint8_t *payload, size_t size, uint8_t maskKey[4]) {
  for (size_t i = 0; i < size; i++) {
    payload[i] ^= maskKey[i % 4];
  }
};
} // namespace XI
