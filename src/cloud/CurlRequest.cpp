#include "CurlRequest.h"
#include "CurlWebClient.h"
#include "CurlRequest.h"

using namespace jm::web::cloud;
using namespace jm::web::cloud::curl;

CurlRequest::CurlRequest(
  RequestType type
  , std::string&& url
  , const std::function<void(WebRequest&)>& onComplete
  , const std::function<void(WebRequest&)>& onError
  , std::map<std::string, std::string>&& params
  , const std::list<std::string>& headers
  , std::string&& content
  , std::shared_ptr<DataSource>&& source
  , CurlWebClient& client)
  : WebRequest(type
    , std::move(url)
    , onComplete
    , onError
    , std::move(params)
    , headers
    , std::move(content)
    , std::move(source)
  )
    , m_client(std::static_pointer_cast<CurlWebClient>(client.shared_from_this()))
    , m_curlHandle(nullptr)
{

}

CurlRequest::~CurlRequest() {
  curl_easy_cleanup(m_curlHandle);
}

std::future<void> CurlRequest::send()
{
  return m_client->send(*this);
}
