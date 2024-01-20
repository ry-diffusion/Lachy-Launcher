#include <cll/event_serializer_extensions.h>
#include <cll/event.h>

using namespace cll;

DefaultExtensions::DefaultExtensions() {
    user.setLocalId("");
    device.setLocalId("");
    device.setDeviceClass("Android.PC");
}

void DefaultExtensions::addTo(EventSerializer& serializer) {
    serializer.addExtension(user);
    serializer.addExtension(os);
    serializer.addExtension(device);
    serializer.addExtension(android);
}


OsInfoExtension::OsInfoExtension() {
    data["ver"] = "1.0";
    data["locale"] = getLocale();
}

std::string OsInfoExtension::getLocale() const {
    std::string cppName = std::locale("").name(); // example: en_US.utf8
    if (cppName.length() < 5)
        return std::string();
    cppName = cppName.substr(0, 5); // cut the suffix
    if (cppName[2] == '_')
        cppName[2] = '-';
    if (cppName[2] != '-') // check for invalid locale
        return std::string();
    return cppName;
}


nlohmann::json AndroidExtension::build(Event const& ev) {
    nlohmann::json data;
    data["ver"] = "1.0";
    data["libVer"] = "3.170921.0";
    auto& tickets = data["tickets"] = nlohmann::json::array();
    for (auto const& id : ev.getIds())
        tickets.push_back(id);
    return data;
}