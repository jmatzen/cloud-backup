#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace jm
{
  std::string base58encode(const uint8_t* input, size_t n);

  template<typename T>
  std::string base58encode(T& input) {
    return input.empty() ? std::string() : base58encode(input.data(), input.size());
  }
}
