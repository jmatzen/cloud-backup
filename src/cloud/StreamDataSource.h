#pragma once

#include "DataSource.h"
#include <fstream>

namespace jm::web::cloud
{
  class StreamDataSource : public DataSource
  {
  public:
    StreamDataSource(std::unique_ptr<std::istream>&& stream)
      : _stream(std::move(stream)) {}

    //////////////////////////////////////////////////////////////////////////

    size_t read(char* p, size_t n) const override;

    DataAttributes getDataAttributes() const override;

    void rewind() override;

    //////////////////////////////////////////////////////////////////////////

  private:
    mutable std::unique_ptr<std::istream> _stream;
  };
}
