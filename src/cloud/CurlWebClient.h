#pragma once

#include <curl/curl.h>
#include "HttpClient.h"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <deque>
#include <map>


namespace jm::web::cloud::curl
{
  class CurlRequest;

  class CurlWebClient
    : public HttpClient
  {
  public:
    CurlWebClient(const Credentials& );

    ~CurlWebClient();

    std::shared_ptr<WebRequest> createWebRequest(
      WebRequest::RequestType requestType,
      std::string&& url,
      const std::function<void(WebRequest&)>& onComplete,
      const std::function<void(WebRequest&)>& onError,
      std::map<std::string, std::string>&& params,
      const std::list<std::string>& headers,
      std::string&& content,
      std::shared_ptr<DataSource>&& source
    ) override;

    void submit(CurlRequest& request);

    std::future<void> send(WebRequest& request) override;

  private:
    static size_t readHandler(char* buf, size_t sz, size_t n, void* s);

    static size_t writeHandler(char* buf, size_t sz, size_t n, void* s);

    static int debugHandler(
      CURL *handle,
      curl_infotype type,
      char *data,
      size_t size,
      void *userptr);
    
    void threadEntryPoint();
    void update();

    std::thread m_thread;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::deque<std::shared_ptr<CurlRequest>> m_queue;
    CURLM* m_multiHandle;
    std::map<CURL*, std::shared_ptr<CurlRequest>> m_requestMap;
    std::shared_ptr<DataSource> m_source;
  };
}
