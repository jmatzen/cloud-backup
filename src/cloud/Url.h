#pragma once

#include <string>
#include <cctype>
#include <list>
#include <string.h>

namespace jm::web::cloud
{
  class Url
  {
  public:

    enum Protocol 
    {
      HTTP, HTTPS
    };

    Url(Protocol p, const std::string host, const std::string& path)
      : m_proto(p)
      , m_host(host)
      , m_path(path) {}

    operator std::string() const {
      std::string result = (m_proto == HTTP ? "http" : "https");
      result.append("://");
      result.append(m_host);
      if (!m_path.empty() && m_path.front() != '/')
        result.append("/");
      result.append(/*urlEncode*/(m_path));
      if (!m_params.empty()) {
        result += '?';
        std::for_each(m_params.begin(), m_params.end(), [&result](auto& p) {
          result += urlEncode(p.first) + "=" + urlEncode(p.second);
          result += '&';
        });
      }
      if (!result.empty() && result.back() == '&')
        result.pop_back();
      return result;
    }

    Url& addParam(const std::string& key, const std::string& value)
    {
      m_params.emplace_back( key, value );
      return *this;
    }

    static std::string urlEncode(const std::string& value) {
      std::string result;
      std::for_each(value.begin(), value.end(), [&result](char ch) {
        if (isalnum(ch)) {
          result.push_back(ch);
        }
        else {
          char buf[16];
          snprintf(buf, 16, "%%%02x", ch);
          result.append(buf);
        }
      });
      return result;
    }

  private:

    Protocol m_proto;
    std::string m_host;
    std::string m_path;
    std::list < std::pair < std::string, std::string>> m_params;
  };
}