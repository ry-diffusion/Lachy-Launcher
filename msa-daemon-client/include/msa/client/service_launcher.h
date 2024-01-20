#pragma once

#include <daemon_utils/daemon_launcher.h>
#include <EnvPathUtil.h>

namespace msa {
namespace client {

class ServiceClient;

class ServiceLauncher : public daemon_utils::daemon_launcher {

private:
    std::string data_path;
    std::string executable_path;

    static std::string getDefaultDataPath() {
        return EnvPathUtil::getDataHome() + "/msa";
    }

public:
    ServiceLauncher(std::string const& executable_path, std::string const& data_path = getDefaultDataPath()) :
            daemon_launcher(data_path + "/service"), data_path(data_path), executable_path(executable_path) {}

    std::vector<std::string> get_arguments() override {
        return {executable_path, "-d", data_path, "-x"};
    }

};

}
}