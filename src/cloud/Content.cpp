#include "Content.h"
#include "StreamDataSource.h"
#include "Exceptions.h"

#include <cassert>

using namespace jm::web::cloud;

Content::Content(const directory_entry& de)
  : _dirent(de)
  , _contentType()
{
  auto stream = std::make_unique<std::ifstream>(de.path().u8string().c_str(), std::ios::binary);
  assert(stream->is_open());
  if (!stream->is_open()) {
    throw FileIOException();
  }
  _source = std::make_shared<StreamDataSource>(std::move(stream));
}

Content::Content(std::unique_ptr<std::istream>&& stream, const std::string& contentType)
  : _dirent()
  , _contentType(contentType)
{
  _source = std::make_shared<StreamDataSource>(std::move(stream));
}
