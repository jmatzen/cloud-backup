#include "CurlWebClient.h"
#include "CurlRequest.h"
#include "DataSource.h"
#include "Credentials.h"
#include <chrono>
#include <algorithm>
#include <cassert>
#include <iostream>

using jm::web::cloud::curl::CurlWebClient;
using jm::web::cloud::curl::CurlRequest;
using jm::web::cloud::HttpClient;
using jm::web::cloud::WebRequest;
using jm::web::cloud::DataSource;
using jm::web::cloud::Credentials;

CurlWebClient::CurlWebClient(const Credentials& credentals)
  : HttpClient(credentals)
  , m_multiHandle(0)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  m_thread = std::thread([this]() {
    threadEntryPoint();
  });
  m_cv.wait(lk);
}

CurlWebClient::~CurlWebClient()
{
  if (m_thread.joinable()) {
    std::unique_lock<std::mutex> lk(m_mutex);
    std::thread thread = std::move(m_thread);
    m_cv.notify_all();
    m_cv.wait(lk);
    thread.join();
  }
  if (m_multiHandle)
  {
    curl_multi_cleanup(m_multiHandle);
  }
}

void CurlWebClient::threadEntryPoint()
{

  m_multiHandle = curl_multi_init();

  {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cv.notify_all();
  }


  while (m_thread.joinable())
  {
    {
      std::unique_lock<std::mutex> lk(m_mutex);
      std::for_each(m_queue.begin(), m_queue.end(), [this](auto& val) {
        submit(*val);
      });
      m_queue.clear();
    }
    update();
  }
  m_cv.notify_all();
}

void CurlWebClient::update()
{
  fd_set fdread;
  fd_set fdwrite;
  fd_set fdexcept;
  FD_ZERO(&fdread);
  FD_ZERO(&fdwrite);
  FD_ZERO(&fdexcept);
  int maxfd;

  int res = curl_multi_fdset(m_multiHandle, &fdread, &fdwrite, &fdexcept, &maxfd);
  if (res != CURLM_OK)
    return;
  if (maxfd == -1) {
    Sleep(10);
  }
  else {
    auto timeout = timeval();
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcept, &timeout);
    if (rc <= 0)
      return;

  }

  int handles;
  curl_multi_perform(m_multiHandle, &handles);

  CURLMsg* msg;
  int msgremain;
  while ((msg = curl_multi_info_read(m_multiHandle, &msgremain))) {
    if (msg->msg == CURLMSG_DONE) {
      auto it = m_requestMap.find(msg->easy_handle);
      assert(it != m_requestMap.end());
      if (it != m_requestMap.end()) {
        auto& requestPtr = it->second;
        long responseCode;
        if (curl_easy_getinfo(it->first, CURLINFO_RESPONSE_CODE, &responseCode) != CURLE_OK) {
          throw std::exception("unexpected failure.");
        }
        requestPtr->setResponseCode(responseCode);
        if (responseCode / 100 != 2) {
          requestPtr->error();
        }
        else {
          requestPtr->complete();
        }
        requestPtr->m_promise.set_value();
        m_requestMap.erase(it);
      }
    }
  }
}

std::shared_ptr<WebRequest> CurlWebClient::createWebRequest(
  WebRequest::RequestType requestType,
  std::string&& url,
  const std::function<void(WebRequest&)>& onComplete,
  const std::function<void(WebRequest&)>& onError,
  std::map<std::string,  std::string>&& params,
  const std::list<std::string>& headers,
  std::string&& content,
  std::shared_ptr<DataSource>&& source
)
{
  return std::make_shared<CurlRequest>(
    requestType,
    std::move(url),
    onComplete,
    onError,
    std::move(params),
    headers,
    std::move(content),
    std::move(source),
    *this);
}

std::future<void> CurlWebClient::send(WebRequest& request)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  auto& curlRequest = std::static_pointer_cast<CurlRequest>(request.shared_from_this());
  m_queue.emplace_back(curlRequest);
  m_cv.notify_all();
  return curlRequest->m_promise.get_future();
}

size_t CurlWebClient::readHandler(char* buf, size_t sz, size_t n, void* s) {
  auto request = reinterpret_cast<CurlRequest*>(s);
  return request->read(buf, sz*n);
}

size_t CurlWebClient::writeHandler(char* buf, size_t sz, size_t n, void* s)
{
  auto request = (CurlRequest*)s;
  request->write(buf, sz*n);
  return sz*n;
}

int CurlWebClient::debugHandler(
  CURL *handle,
  curl_infotype type,
  char *data,
  size_t size,
  void *userptr)
{
  std::cerr << data << std::endl;
  return CURLE_OK;
}


void CurlWebClient::submit(CurlRequest& request)
{
  auto handle = curl_easy_init();
  curl_easy_setopt(handle, CURLOPT_URL, request.getUrl().c_str());
  curl_easy_setopt(handle, CURLOPT_READDATA, &request);
  curl_easy_setopt(handle, CURLOPT_READFUNCTION, &CurlWebClient::readHandler);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &request);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &CurlWebClient::writeHandler);

  curl_slist*list = 0;
  auto& headers = request.getHeaders();
  std::for_each(headers.begin(), headers.end(), [&list](auto& val) {
    list = curl_slist_append(list, val.c_str());
  });
  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, list);

  //curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
  //curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION, &CurlWebClient::DebugHandler);
  //curl_easy_setopt(handle, CURLOPT_DEBUGDATA, &request);
  switch (request.getRequestType()) {
  case WebRequest::REQUEST_TYPE_GET:
    break;
  case WebRequest::REQUEST_TYPE_POST:
    curl_easy_setopt(handle, CURLOPT_POST, 1);
    break;
  default:
    throw std::exception("invalid request type");
  }

  std::string data;
  request.forEachParam([handle,&data](auto& p)
  {
    auto e = curl_easy_escape(handle, p.first.c_str(), p.first.length());
    data += e;
    data += "=";
    curl_free(e);
    e = curl_easy_escape(handle, p.second.c_str(), p.second.length());
    data += e;
    curl_free(e);
    data += "&";
  });
  if (!data.empty()) {
    data.pop_back();
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, data.length());
    curl_easy_setopt(handle, CURLOPT_COPYPOSTFIELDS, data.c_str());
  }


  curl_multi_add_handle(m_multiHandle, handle);

  int handles;
  curl_multi_perform(m_multiHandle, &handles);


  m_requestMap.emplace(
    handle,
    std::static_pointer_cast<CurlRequest>(request.shared_from_this()));
}


//////////////////////////////////////////////////////////////////////////

std::shared_ptr<HttpClient> HttpClient::createWebClient(const Credentials& credentals)
{
  return std::make_shared<CurlWebClient>(credentals);
}

HttpClient::HttpClient(const Credentials& c)
  : credentials_(c.shared_from_this())
{

}

