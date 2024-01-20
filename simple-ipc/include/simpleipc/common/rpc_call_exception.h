#pragma once

#include <nlohmann/json.hpp>
#include "message/error_code.h"

namespace simpleipc {

class rpc_call_exception_interface : public std::exception {

public:
    virtual rpc_error_code code() const = 0;
    virtual nlohmann::json const& data() const = 0;

};

class rpc_call_exception : public rpc_call_exception_interface {

private:
    rpc_error_code _code;
    std::string _text;
    nlohmann::json _data;

public:
    rpc_call_exception(rpc_error_code code, std::string const& text, nlohmann::json data = nullptr) :
            _code(code), _text(std::move(text)), _data(std::move(data)) {
    }

    rpc_call_exception(rpc_error_code code, rpc_error_string_translator const& tr, nlohmann::json data = nullptr) :
            _code(code), _text(tr(code)), _data(std::move(data)) {
    }


    const char* what() const noexcept override {
        return _text.c_str();
    }

    rpc_error_code code() const override {
        return _code;
    }
    nlohmann::json const& data() const override {
        return _data;
    }

};

}