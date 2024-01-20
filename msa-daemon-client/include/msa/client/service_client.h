#pragma once

#include <simpleipc/client/service_client.h>
#include <simpleipc/client/rpc_call.h>
#include <daemon_utils/launchable_service_client.h>
#include "account.h"
#include "token.h"

namespace msa {
namespace client {

class ServiceClient : public daemon_utils::launchable_service_client {

public:
    explicit ServiceClient(std::string const& path) : launchable_service_client(path) {}
    explicit ServiceClient(daemon_utils::daemon_launcher& launcher) : launchable_service_client(launcher) {}


    /**
     * List the accounts that the user has signed in with.
     * You probably should use pickAccount instead to let the user select an account, which also lets the user sign in
     * if no accounts are registered, and then store the account id.
     * @return a rpc_call that contains a list of basic account information (without tokens)
     */
    simpleipc::client::rpc_call<std::vector<BaseAccountInfo>> getAccounts();

    /**
     * Remove an account with the specified CID.
     * @param cid the account id (CID) of the account to add
     * @param puid the passport id of the account
     * @return a void rpc_call
     */
    simpleipc::client::rpc_call<void> addAccount(std::string const& cid, std::string const& puid,
            std::string const& username, std::string const& token);

    /**
     * Remove an account with the specified CID.
     * @param cid the account id (CID) of the account to remove
     * @return a void rpc_call
     */
    simpleipc::client::rpc_call<void> removeAccount(std::string const& cid);

    /**
     * Opens an UI that lets the user select an account.
     * @param clientId application's client id
     * @param cobrandId brand id (used for the login page style, optional)
     * @return a rpc_call that contains an account id (CID)
     */
    simpleipc::client::rpc_call<std::string> pickAccount(std::string const& clientId,
                                                         std::string const& cobrandId = std::string());

    /**
     * Opens an UI that lets the user select an account.
     * @param cid the account id (CID) of the account to remove
     * @param scope the scope to request the token for
     * @param clientId application's client id
     * @param silent if true, the request will not open an UI
     * @return a rpc_call that contains the token
     */
    simpleipc::client::rpc_call<std::shared_ptr<Token>> requestToken(std::string const& cid, SecurityScope const& scope,
                                                                     std::string const& clientId, bool silent = false);

};

}
}