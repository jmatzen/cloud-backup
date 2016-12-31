#pragma once

#include <memory>
#include <string>
#include <iosfwd>

namespace jm::web::cloud
{
  class Credentials
    : public std::enable_shared_from_this<Credentials>
  {
  public:
    enum Type
    {
      kTypeServiceAccount
    };

    static std::shared_ptr<Credentials> load(std::istream& stream);

    Credentials(Type type,
      std::string const & projectId,
      std::string const & privateKeyId,
      std::string const & privateKey,
      std::string const & clientEmail,
      std::string const & clientId,
      std::string const & authUri,
      std::string const & tokenUri,
      std::string const & authProviderx509CertUrl,
      std::string const & clientx509CertUrl)
      : _type(type)
      , _projectId(projectId)
      , _privateKeyId(privateKeyId)
      , _privateKey(privateKey)
      , _clientEmail(clientEmail)
      , _clientId(clientId)
      , _authUri(authUri)
      , _tokenUri(tokenUri)
      , _authProviderx509CertUrl(authProviderx509CertUrl)
      , _clientx509CertUrl(clientx509CertUrl)
    {
    }

    const std::string& GetPrivateKey() const { return _privateKey; }
    const std::string& GetEmail() const { return _clientEmail; }

    static Type GetTypeFromString(const std::string& s)
    {
      if (!stricmp(s.c_str(), "service_account"))
        return kTypeServiceAccount;
      throw std::exception("unable to convert type.");
    }

  private:
    Type _type;
    std::string _projectId;
    std::string _privateKeyId;
    std::string _privateKey;
    std::string _clientEmail;
    std::string _clientId;
    std::string _authUri;
    std::string _tokenUri;
    std::string _authProviderx509CertUrl;
    std::string _clientx509CertUrl;
  };
}
