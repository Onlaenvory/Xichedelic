#pragma once

#include <cstdint>
#include <random>

namespace XI {
  class Randomizer {
    public:
      static uint32_t key32_t() {
        thread_local std::mt19937 key(std::random_device{}());
        std::uniform_int_distribution<uint32_t> range(0, 0xFFFFFFFF);
        return range(key);
      }
  };
} // namespace XI
