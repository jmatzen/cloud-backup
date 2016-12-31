#pragma once

#include "HttpClient.h"
#include "DataSource.h"

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <exception>
#include <map>
#include <list>

namespace jm::web::cloud
{
  class RequestBuilder
  {
  public:
    RequestBuilder& setType(WebRequest::RequestType type) {
      m_type = type;
      return *this;
    }

    RequestBuilder& setUrl(const std::string& url) {
      m_url = url;
      return *this;
    }

    RequestBuilder& setCompletion(const std::function<void(WebRequest&)>& fn) {
      m_callback = fn;
      return *this;
    }

    RequestBuilder& setError(const std::function<void(WebRequest&)>& fn) {
      m_error = fn;
      return *this;
    }

    RequestBuilder& setFormParameter(const std::string& key, const std::string& value) {
      if (!m_params.emplace(key, value).second) {
        throw std::exception("insert failed");
      }
      return *this;
    }

    RequestBuilder& setHeader(const std::string& key, const std::string& value, bool cond = true) {
      if (cond)
        m_headers.emplace_back(key + ": " + value);
      return *this;
    }

    RequestBuilder& setClient(HttpClient& client);

    RequestBuilder& setContent(std::string&& buf) {
      m_content = std::move(buf);
      return *this;
    }

    RequestBuilder& setDataSource(const std::shared_ptr<DataSource>& source) {
      source->rewind();
      m_source = source;
      return *this;
    }

    std::shared_ptr<WebRequest> build();

  private:
    WebRequest::RequestType m_type;
    std::string m_url;
    std::function<void(WebRequest&)> m_callback;
    std::function<void(WebRequest&)> m_error;
    std::map<std::string, std::string> m_params;
    std::shared_ptr<HttpClient> m_client;
    std::list<std::string> m_headers;
    std::string m_content;
    std::shared_ptr<DataSource> m_source;
  };

}
