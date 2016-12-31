#include <cryptopp/cryptlib.h>
#include <cryptopp/rsa.h>
#include <cryptopp/asn.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/pssr.h>
#include <cryptopp/osrng.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "HttpClient.h"
#include "RequestBuilder.h"
#include "Url.h"
#include "StreamDataSource.h"
#include "Base58.h"
#include "Credentials.h"
#include "Content.h"

#include <iostream>
#include <cassert>
#include <experimental/filesystem>


using namespace jm::web::cloud;
namespace fs = std::experimental::filesystem;
namespace c = CryptoPP;
namespace rj = rapidjson;

struct AuthorizationFailedException {};


void HandleLoginRequired(HttpClient& client/*, const fs::directory_entry& path*/)
{
  auto creds = client.GetCredentials().shared_from_this();

  c::RSA::PrivateKey pk;
  pk.Load(
    c::StringSource(
      creds->GetPrivateKey().c_str()
      , true
      , new c::Base64Decoder));

  std::string wtHeader;
  c::StringSource("{\"alg\":\"RS256\",\"typ\":\"JWT\"}"
    , true
    , new c::Base64URLEncoder(
      new c::StringSink(wtHeader)));

  rj::Document d;
  d.SetObject();
  // write a value for the token scope
  d.AddMember("scope", "https://www.googleapis.com/auth/devstorage.read_write", d.GetAllocator());
  // issuer is the e-mail address from the credentials files
  d.AddMember("iss", rj::GenericStringRef<char>(creds->GetEmail().c_str()), d.GetAllocator());
  // audience is the auth url
  d.AddMember("aud", "https://www.googleapis.com/oauth2/v4/token", d.GetAllocator());
  // need to write the expiration time 
  d.AddMember("exp", time(0) + 120, d.GetAllocator());
  // and the iat - these are used to defeat replay attacks
  d.AddMember("iat", time(0), d.GetAllocator());
  // allocate a string buffer on the stack
  rj::StringBuffer sb;
  d.Accept(rj::PrettyWriter<rj::StringBuffer>(sb));
  std::string jwtContent = sb.GetString();

  std::string wtClaimSet;
  c::StringSource(jwtContent
    , true
    , new c::Base64URLEncoder(
      new c::StringSink(wtClaimSet)));

  std::string webToken = wtHeader + "." + wtClaimSet;

  c::AutoSeededRandomPool randomPool;

  std::string signatureBase64;
  //std::string signature;
  c::RSASS<c::PKCS1v15, c::SHA256>::Signer signer(pk);
  c::StringSource(webToken, true,
    new c::SignerFilter(
      randomPool,
      signer,
      new c::Base64URLEncoder(
        new c::StringSink(signatureBase64))));

  webToken += "." + signatureBase64;

  std::string token;
  int statusCode = 0;

  auto future = RequestBuilder()
    .setClient(client)
    .setType(WebRequest::REQUEST_TYPE_POST)
    .setUrl("https://www.googleapis.com/oauth2/v4/token")
    .setFormParameter("assertion", webToken)
    .setFormParameter("grant_type", "urn:ietf:params:oauth:grant-type:jwt-bearer")
    .setCompletion([&token, &statusCode](auto& request)
  {
    statusCode = request.getStatusCode();
    if (statusCode != 200)
      return;
    rj::Document d;
    d.Parse(request.getBuffer().c_str());
    token += d["token_type"].GetString();
    token += " ";
    token += d["access_token"].GetString();
  })
    .build()->send();

  future.get();

  if (statusCode != 200)
    throw AuthorizationFailedException();

  client.SetState(token);
}

void GetFileState(HttpClient& client, Content& content)
{
  auto dataSource = content.GetSource();

  auto attribs = dataSource->getDataAttributes();

  auto& hash = attribs.hash;

  auto hashstr = jm::base58encode(hash);

  std::string urlPath = "/storage/v1/b/ef0e4f8f-2d34-4200-b98f-7d57262d87a8/o/";
  urlPath += hashstr;

  int statusCode = 0;

  auto builder = RequestBuilder()
    .setClient(client)
    .setType(WebRequest::REQUEST_TYPE_GET)
    .setUrl(Url(Url::HTTPS, "www.googleapis.com",urlPath))
    .setError([&client](auto& request) 
  {
    HandleLoginRequired(client);
  })
    .setCompletion([&statusCode](auto& request) 
  {
    statusCode = request.getStatusCode();
    std::cout << "complete " << request.getStatusCode() << " " << request.getUrl() << std::endl;
    std::string buf = request.getBuffer();
    std::cout << buf << std::endl;
  });


  const std::string& httpClientState = client.GetState();
  if (!httpClientState.empty())
    builder.setHeader("Authorization", httpClientState);


  auto future = builder.build()->send();
  future.get();
  switch (statusCode) {
  case 401:
    HandleLoginRequired(client);
    GetFileState(client, content);
    break;
  }
}

void UploadFile(HttpClient& client, std::unique_ptr<Content>&& content)
{
  GetFileState(client, *content);

  content->GetSource()->rewind();

  int statusCode = 0;

  std::string mimeType = content->GetContentType();
  
  if (mimeType.empty())
    mimeType = "application/octet-stream";

  auto& dirEntry = content->GetDirEntry();
  if (dirEntry.path().extension() == ".jpg") { // case sensitivity?
    mimeType = "image/jpeg";
  }

  auto dataSource = content->GetSource();

  auto attribs = dataSource->getDataAttributes();

  auto& hash = attribs.hash;

  if (attribs.size > 1024 * 1024)
    return;

  auto hashstr = jm::base58encode(hash);

  std::cout << hashstr << " " << dirEntry.path().u8string() << std::endl;

  auto builder = RequestBuilder()
    .setClient(client)
    .setType(WebRequest::REQUEST_TYPE_POST)
    .setUrl(Url(Url::HTTPS,
      "www.googleapis.com",
      "/upload/storage/v1/b/ef0e4f8f-2d34-4200-b98f-7d57262d87a8/o")
      .addParam("uploadType", "media")
      .addParam("name", hashstr.c_str()))
    .setHeader("Content-type", mimeType)
    .setHeader("Transfer-Encoding", "chunked")
    .setDataSource(dataSource)
    .setCompletion([&client,&dirEntry,&statusCode](auto& request)
  {
    std::cout << "complete " << request.getStatusCode() << " " << request.getUrl() << std::endl;
    statusCode = request.getStatusCode();
  });

  const std::string& httpClientState = client.GetState();

  if (!httpClientState.empty())
  {
    builder.setHeader("Authorization", httpClientState);
  }

  auto future = builder.build()->send();

  future.get();

  switch (statusCode) {
  case 401:
    HandleLoginRequired(client);
    UploadFile(client, std::move(content));
    break;
  }

}