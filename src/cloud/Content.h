#pragma once

#include <experimental/filesystem>

namespace jm::web::cloud
{
  class StreamDataSource;

  class Content
  {
    enum State
    {
      STATE_EXIST_CHECK,
      STATE_NEED_AUTH,
      STATE_UPLOADING
    };

    typedef std::experimental::filesystem::directory_entry directory_entry;

  public:
    explicit Content(const directory_entry& de);
    explicit Content(std::unique_ptr<std::istream>&&, const std::string& contentType);

    const directory_entry& GetDirEntry() const { return _dirent; }
    std::shared_ptr<StreamDataSource> GetSource() const { return _source; }
    const std::string& GetContentType() const { return _contentType; }
  private:
    State _state;
    directory_entry _dirent;
    std::shared_ptr<StreamDataSource> _source;
    std::string _contentType;
  };
}
