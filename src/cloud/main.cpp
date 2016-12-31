
#include <memory>
#include <string>
#include <fstream>
#include <exception>
#include <cstring>
#include <sstream>
#include <ctime>
#include <thread>
#include <winsock2.h>
#include <mutex>
#include <iostream>
#include <experimental/filesystem>

#include "Credentials.h"
#include "HttpClient.h"
#include "WebRequest.h"
#include "RequestBuilder.h"
#include "Url.h"
#include "ThreadPoolExecutor.h"
#include "Content.h"
#include "Base58.h"
#include "StreamDataSource.h"

namespace fs = std::experimental::filesystem;

using jm::web::cloud::Credentials;
using jm::web::cloud::WebRequest;
using jm::web::cloud::RequestBuilder;
using jm::web::cloud::HttpClient;
using jm::web::cloud::Url;
using jm::web::cloud::Content;

void UploadFile(HttpClient& client, std::unique_ptr<Content>&& path);

namespace
{
  const int MAX_FILES = 500;

}
int main() {

  auto wsaData = WSADATA();
  WSAStartup(MAKEWORD(2, 1), &wsaData);

  std::ifstream credentialStream("C:/depot/StampyTube-4391e5fb4f87.json");
  if (!credentialStream) {
    throw std::exception("unable to open credentials file");
  }

  auto credentials = Credentials::load(credentialStream);

  typedef fs::recursive_directory_iterator::value_type fileinfo_type;

  // everything we need for multi-consumer single-producer queue
  std::mutex m;
  std::condition_variable cv1, cv2;
  std::vector<std::thread> threads;
  std::deque<std::unique_ptr<Content>> queue;
  std::atomic_bool running = true;


  for (int i = 0; i < 1; ++i) {
    threads.emplace_back([&]() {
      auto webClient = jm::web::cloud::HttpClient::createWebClient(*credentials);
      for (;;)
      {
        std::unique_lock<std::mutex> lk(m);

        while (running && queue.empty()) {
          cv2.notify_one();
          cv1.wait(lk);
        }

        if (!running) break;

        auto content = std::move(queue.front());
        queue.pop_front();
        std::cout << std::this_thread::get_id() << " " << content->GetDirEntry().path().u8string() << std::endl;
        lk.unlock();

        UploadFile(*webClient, std::move(content));
      }
    });
  }

  auto manifest = std::make_unique<std::stringstream>();
  int count = 0;


  for (auto& p : fs::recursive_directory_iterator(R"%(H:\older pictures)%")) {

    if (!fs::is_regular_file(p)) {
      continue;
    }

    std::cout << p.path().u8string() << std::endl;
    if (p.path().u8string().find(".jpg") == std::string::npos)
      continue;
    auto content = std::make_unique<Content>(p);
    //if (content->GetSource()->getDataAttributes().size > 1024 * 1024)
    //  continue;
    if (++count > MAX_FILES)
      break;

    std::unique_lock<std::mutex> lk(m);
    while (queue.size() > 10) {
      cv2.wait(lk);
    }


    auto& contentHash = content->GetSource()->getDataAttributes().hash;
    (*manifest) << jm::base58encode(contentHash) << " " << content->GetDirEntry().path().u8string() << std::endl;
    queue.push_back(std::move(content));
    cv1.notify_one();
  }

  std::ofstream("manifest.txt") << manifest->str();

  auto manifestContent = std::make_unique<Content>(std::move(manifest), "text/plain");
  std::string manifestId = jm::base58encode(
    manifestContent->GetSource()->getDataAttributes().hash);

  {
    std::unique_lock<std::mutex> lk(m);
    while (!queue.empty()) {
      cv2.wait(lk);
    }
  }

  std::cout << "uploading manifest " << manifestId << std::endl;

  {
    std::unique_lock<std::mutex> lk(m);
    queue.push_back(std::move(manifestContent));
    cv1.notify_one();
    cv2.wait(lk);
  }

  {
    std::unique_lock<std::mutex> lk(m);
    running = false;
    cv1.notify_all();
  }

  std::for_each(
    threads.begin(), 
    threads.end(), 
    std::bind(&std::thread::join, std::placeholders::_1));

  std::cout << "manifest is " << manifestId << std::endl;

}
