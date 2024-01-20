#include "encodings.h"
#include "encoding_json.h"
#include "encoding_json_cbor.h"

using namespace simpleipc::encoding;

encodings encodings::instance;

encodings::encodings() {
    add_encoding(new json());
    add_encoding(new json_cbor());

    default_encoding = encodings_map["json"].get();
}

void encodings::add_encoding(encoding* enc) {
    encodings_map[enc->name()] = std::unique_ptr<encoding>(enc);
}

encoding* encodings::get_default_encoding() {
    return instance.default_encoding;
}

encoding* encodings::get_encoding_by_name(std::string const& name) {
    auto it = instance.encodings_map.find(name);
    if (it == instance.encodings_map.end())
        return nullptr;
    return it->second.get();
}

std::vector<std::string> encodings::get_preferred_encodings() {
    return {"json_cbor", "json"};
}