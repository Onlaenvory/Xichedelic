#pragma once

#include <openssl/rand.h>
#include <cstdint>
#include <random>

namespace XI {
static uint32_t Key32() {
  thread_local std::mt19937 key(std::random_device{} ());
  std::uniform_int_distribution<uint32_t> range(0x00000000, 0xFFFFFFFF);
  return range(key);
}

static std::string Key128Base64() {
  unsigned char buffer[16];
  unsigned char base64_out[32];

  RAND_bytes(buffer, sizeof(buffer));
  int length = EVP_EncodeBlock(base64_out, buffer, sizeof(buffer));

  return std::string(reinterpret_cast<char *>(base64_out), static_cast<size_t>(length));
}
} // namespace XI
