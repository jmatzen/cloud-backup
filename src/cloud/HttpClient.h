#pragma once

#include <map>
#include <string>
#include <memory>
#include <list>

#include "WebRequest.h"

namespace jm::web::cloud
{
  class RequestBuilder;
  class DataSource;
  class State;
  class Credentials;

  class HttpClient
    : public std::enable_shared_from_this<HttpClient>
  {
  public:

    HttpClient(const Credentials& c);

    virtual ~HttpClient() {}

    virtual std::shared_ptr<WebRequest> createWebRequest(
      WebRequest::RequestType requestType,
      std::string&& url,
      const std::function<void(WebRequest&)>& onComplete,
      const std::function<void(WebRequest&)>& onError,
      std::map<std::string, std::string>&& params,
      const std::list<std::string>& headers,
      std::string&& content,
      std::shared_ptr<DataSource>&& source
      ) = 0;

    virtual std::future<void> send(WebRequest& request) = 0;

    static std::shared_ptr<HttpClient> createWebClient(const Credentials& credentals);

    void SetState(const std::string& state) {
      state_ = state;
    }

    const std::string& GetState() const { return state_; }

    const Credentials& GetCredentials() const { return *credentials_; }

  private:
    std::string state_;
    std::shared_ptr<const Credentials> credentials_;
  };
}
