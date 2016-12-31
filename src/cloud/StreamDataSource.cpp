#include <cryptopp/sha.h>

#include "StreamDataSource.h"


using namespace jm::web::cloud;
namespace c = CryptoPP;


size_t StreamDataSource::read(char* p, size_t n) const
{
  _stream->read(p, n);
  return static_cast<size_t>(_stream->gcount());
}

DataAttributes StreamDataSource::getDataAttributes() const
{
  auto p = _stream->tellg();
  _stream->seekg(0, std::ios::end);
  auto sz = _stream->tellg();
  _stream->seekg(0);
  c::SHA256 sha;
  auto buf = std::make_unique<uint8_t[]>(1024 * 1024);
  for (;;) {
    _stream->read((char*)buf.get(), 1024 * 1024);
    size_t bread = static_cast<size_t>(_stream->gcount());
    if (bread == 0) break;
    sha.Update(buf.get(), bread);
  }
  Hash hash;
  sha.Final(hash.data());
  _stream->clear();
  _stream->seekg(p);
  return DataAttributes{ hash, sz };
}

void StreamDataSource::rewind()
{
  _stream->clear();
  _stream->seekg(0);
}
