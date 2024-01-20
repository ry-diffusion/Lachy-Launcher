#include <msa/client/token.h>
#include <msa/client/legacy_token.h>
#include <msa/client/compact_token.h>

using namespace msa::client;

Token::Token(nlohmann::json const& data) {
    auto scopeData = data.value("scope", nlohmann::json());
    securityScope = {scopeData.value("address", ""), scopeData.value("policy_ref", "")};
    createTime = std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(data.value("created", 0LL)));
    expireTime = std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(data.value("expires", 0LL)));
}

std::shared_ptr<Token> Token::fromJson(nlohmann::json const& data) {
    auto tokenType = data["type"];

    if (tokenType == "urn:passport:legacy")
        return std::make_shared<LegacyToken>(data);
    else if (tokenType == "urn:passport:compact")
        return std::make_shared<CompactToken>(data);
    throw std::runtime_error("Invalid token type");
}