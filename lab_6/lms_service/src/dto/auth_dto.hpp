#pragma once

#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace lms_service::dto {

struct LoginRequestDto {
    std::string login;
    std::string password;
};

struct LoginResponseDto {
    std::string token;
    std::string token_type;
    std::string login;
    std::string role;
};

inline LoginRequestDto ParseLoginRequest(const userver::formats::json::Value& json) {
    return LoginRequestDto{
        .login = json["login"].As<std::string>(""),
        .password = json["password"].As<std::string>(""),
    };
}

inline userver::formats::json::Value ToJson(const LoginResponseDto& dto) {
    userver::formats::json::ValueBuilder builder;

    builder["token"] = dto.token;
    builder["token_type"] = dto.token_type;
    builder["login"] = dto.login;
    builder["role"] = dto.role;
    
    return builder.ExtractValue();
}

}  // namespace lms_service::dto