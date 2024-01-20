#include <msa/client/service_client.h>

using namespace msa::client;

simpleipc::client::rpc_call<std::vector<BaseAccountInfo>> ServiceClient::getAccounts() {
    return simpleipc::client::rpc_call<std::vector<BaseAccountInfo>>(
            rpc("msa/get_accounts", nullptr), [](nlohmann::json const& d) {
                std::vector<BaseAccountInfo> ret;
                for (auto const& a : d["accounts"])
                    ret.push_back(BaseAccountInfo::fromJson(a));
                return ret;
            });
}

simpleipc::client::rpc_call<void> ServiceClient::addAccount(std::string const& cid, std::string const& puid,
        std::string const& username, std::string const& token) {
    nlohmann::json data;
    data["cid"] = cid;
    data["puid"] = puid;
    data["username"] = username;
    data["token"] = token;
    return simpleipc::client::rpc_call<void>(rpc("msa/add_account", data));
}

simpleipc::client::rpc_call<void> ServiceClient::removeAccount(std::string const& cid) {
    nlohmann::json data;
    data["cid"] = cid;
    return simpleipc::client::rpc_call<void>(rpc("msa/remove_account", data));
}

simpleipc::client::rpc_call<std::string> ServiceClient::pickAccount(std::string const& clientId,
                                                                    const std::string& cobrandId) {
    nlohmann::json data;
    if (!clientId.empty())
        data["client_id"] = clientId;
    if (!cobrandId.empty())
        data["cobrandid"] = cobrandId;
    return simpleipc::client::rpc_call<std::string>(
            rpc("msa/pick_account", data), [](nlohmann::json const& d) {
                return d["cid"].get<std::string>();
            });
}

simpleipc::client::rpc_call<std::shared_ptr<Token>> ServiceClient::requestToken(
        std::string const& cid, SecurityScope const& scope, std::string const& clientId, bool silent) {
    nlohmann::json data;
    data["cid"] = cid;
    data["scope"]["address"] = scope.address;
    data["scope"]["policy_ref"] = scope.policyRef;
    if (!clientId.empty())
        data["client_id"] = clientId;
    if (silent)
        data["silent"] = true;

    return simpleipc::client::rpc_call<std::shared_ptr<Token>>(
            rpc("msa/request_token", data), [](nlohmann::json const& d) {
                return Token::fromJson(d);
            });
}