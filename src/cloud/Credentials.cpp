#include "Credentials.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <string>
#include <istream>
#include <sstream>

namespace rj = rapidjson;

using jm::web::cloud::Credentials;

std::shared_ptr<Credentials> Credentials::load(std::istream& stream) {
  // get stream size
  std::size_t size = static_cast<std::size_t>(stream.seekg(0, std::ios::end).tellg());
  // rewind stream
  stream.seekg(0);
  // allocate buffer
  auto buf = std::make_unique<char[]>(size + 1);
  // load file
  stream.read(buf.get(), size);
  buf[size] = 0; // null terminate
  rj::Document d;
  if (d.ParseInsitu(buf.get()).HasParseError())
    throw std::exception("failed to parse credentials");

  std::stringstream s(d["private_key"].GetString());
  std::string privateKey;
  // strip off first line
  std::getline(s, std::string());
  // read
  for (std::string tmp; std::getline(s, tmp), s; privateKey.append(tmp));
  // strip tail
  privateKey.erase(privateKey.find('-'));


  return std::make_shared<Credentials>(
    Credentials::GetTypeFromString(d["type"].GetString()),
    d["project_id"].GetString(),
    d["private_key_id"].GetString(),
    privateKey,
    d["client_email"].GetString(),
    d["client_id"].GetString(),
    d["auth_uri"].GetString(),
    d["token_uri"].GetString(),
    d["auth_provider_x509_cert_url"].GetString(),
    d["client_x509_cert_url"].GetString());
}
