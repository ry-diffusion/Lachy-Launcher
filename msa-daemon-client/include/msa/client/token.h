#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include "security_scope.h"

namespace msa {
namespace client {

enum class TokenType {
    Legacy, Compact
};

class Token {

public:
    using TimePoint = std::chrono::system_clock::time_point;

protected:
    SecurityScope securityScope;
    TimePoint createTime, expireTime;

    Token(nlohmann::json const& data);

public:
    Token() {}

    Token(SecurityScope const& scope, TimePoint create, TimePoint expire) : securityScope(scope), createTime(create),
                                                                            expireTime(expire) {}

    virtual ~Token() = default;

    virtual TokenType getType() const = 0;

    SecurityScope const& getSecurityScope() const { return securityScope; }

    TimePoint getCreatedTime() const { return createTime; }

    TimePoint getExpiresTime() const { return expireTime; }


    static std::shared_ptr<Token> fromJson(nlohmann::json const& data);

};

template<typename Ret>
Ret& token_cast(Token&);

template <typename Ret>
std::shared_ptr<Ret> token_pointer_cast(std::shared_ptr<Token>);

template<typename Ret>
Ret const& token_cast(Token const&);

}
}