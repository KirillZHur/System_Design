#pragma once

#include <cstdint>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

#include "../models/user.hpp"

namespace lms_service::dto {

struct RegisterUserRequestDto {
    std::string login;
    std::string password;
    std::string first_name;
    std::string last_name;
    std::string role;
};

struct UserResponseDto {
    std::int64_t id{0};
    std::string login;
    std::string first_name;
    std::string last_name;
    std::string role;
};

inline RegisterUserRequestDto ParseRegisterUserRequest(const userver::formats::json::Value& json) {
    return RegisterUserRequestDto{
        .login = json["login"].As<std::string>(""),
        .password = json["password"].As<std::string>(""),
        .first_name = json["first_name"].As<std::string>(""),
        .last_name = json["last_name"].As<std::string>(""),
        .role = json["role"].As<std::string>(""),
    };
}

inline UserResponseDto ToUserResponseDto(const lms_service::models::User& user) {
    return UserResponseDto{
        .id = user.id,
        .login = user.login,
        .first_name = user.first_name,
        .last_name = user.last_name,
        .role = user.role,
    };
}

inline userver::formats::json::Value ToJson(const UserResponseDto& dto) {
    userver::formats::json::ValueBuilder builder;
    
    builder["id"] = dto.id;
    builder["login"] = dto.login;
    builder["first_name"] = dto.first_name;
    builder["last_name"] = dto.last_name;
    builder["role"] = dto.role;

    return builder.ExtractValue();
}

}  // namespace lms_service::dto