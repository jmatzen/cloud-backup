#pragma once

#include <memory>
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <list>
#include <future>

#include "DataSource.h"

namespace jm::web::cloud
{

  class WebRequest
    : public std::enable_shared_from_this<WebRequest>
  {
  public:
    virtual ~WebRequest() {}


    enum RequestType
    {
      REQUEST_TYPE_POST,
      REQUEST_TYPE_GET,
    };

    const std::string& getUrl() const { return m_url; }

    RequestType getRequestType() const { return m_type; }

    template<typename Fn> 
    void forEachParam(const Fn& f) {
      std::for_each(m_params.begin(), m_params.end(), f);
    }

    virtual std::future<void> send() = 0;

    const std::string& getBuffer() const { return m_buf; }

    const std::list<std::string>& getHeaders() const { return m_headers; }

    int getStatusCode() const { return m_statusCode; }

  protected:
    WebRequest(
      RequestType type, 
      std::string&& url, 
      const std::function<void(WebRequest&)>& complete,
      const std::function<void(WebRequest&)>& error,
      std::map<std::string, std::string>&& params,
      const std::list<std::string>& headers,
      std::string&& content,
      std::shared_ptr<DataSource>&& source
      )
      : m_type(type)
      , m_url(std::move(url))
      , m_params(std::move(params))
      , m_complete(complete)
      , m_error(error)
      , m_headers(headers)
      , m_content(std::move(content))
      , m_source(std::move(source))
    {

    }

    void complete() {
      if (m_complete) 
        m_complete(*this);
    }

    void error() {
      if (m_error)
        m_error(*this);
    }

    void write(const char* p, size_t n) {
      m_buf.insert(m_buf.end(), p, p + n);
    }

    size_t read(char* p, size_t n) {
      return m_source->read(p, n);
    }

    void setResponseCode(int code) {
      m_statusCode = code;
    }

  private:
    RequestType m_type;
    std::string m_url;
    std::map<std::string, std::string> m_params;
    std::function<void(WebRequest&)> m_complete;
    std::function<void(WebRequest&)> m_error;
    std::string m_buf;
    int m_statusCode;
    std::list<std::string> m_headers;
    std::string m_content;
    std::shared_ptr<DataSource> m_source;
  };
}
