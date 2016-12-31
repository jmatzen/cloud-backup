#include "RequestBuilder.h"

using namespace jm::web::cloud;


RequestBuilder& RequestBuilder::setClient(HttpClient& client) {
  m_client = client.shared_from_this();
  return *this;
}


std::shared_ptr<WebRequest> RequestBuilder::build()
{
  return m_client->createWebRequest(
    m_type, 
    std::move(m_url),
    m_callback,
    m_error,
    std::move(m_params),
    m_headers,
    std::move(m_content),
    std::move(m_source)
    );
}