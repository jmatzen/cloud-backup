#pragma once

#include <memory>
#include <array>
#include <tuple>

namespace jm::web::cloud
{
  typedef std::array<uint8_t, 32> Hash;

  struct DataAttributes {
    Hash hash;
    std::streamsize size;
  };

  class DataSource
    : public std::enable_shared_from_this<DataSource>
  {
  public:
    virtual size_t read(char* p, size_t n) const = 0;
    virtual DataAttributes getDataAttributes() const = 0;
    virtual void rewind() = 0;
  };
}
