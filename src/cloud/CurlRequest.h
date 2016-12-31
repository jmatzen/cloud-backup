#pragma once

#include "WebRequest.h"
#include "DataSource.h"
#include <curl/curl.h>
#include <list>
#include <string>


namespace jm::web::cloud::curl
{
  class CurlWebClient;

  class CurlRequest
    : public WebRequest
  {
  public:
    CurlRequest(
      RequestType type
      , std::string&& url
      , const std::function<void(WebRequest&)>& complete
      , const std::function<void(WebRequest&)>& error
      , std::map<std::string, std::string>&& params
      , const std::list<std::string>& headers
      , std::string&& content
      , std::shared_ptr<DataSource>&& source
      , CurlWebClient& client
    );

    ~CurlRequest();

    CURL* getHandle() const { return m_curlHandle; }

    void setHandle(CURL* handle) {
      m_curlHandle = handle;
    }

    std::future<void> send() override;

    

  private:

    std::shared_ptr<CurlWebClient> m_client;
    CURL* m_curlHandle;
    friend class CurlWebClient;
    std::promise<void> m_promise;
    std::shared_ptr<DataSource> m_source;
  };

}
