#pragma once

#include <cstdint>

#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace lms_service::dto {

struct EnrollUserRequestDto {
    std::int64_t user_id{0};
};

struct EnrollmentResponseDto {
    std::int64_t user_id{0};
    std::int64_t course_id{0};
    std::string status;
};

inline EnrollUserRequestDto ParseEnrollUserRequest(const userver::formats::json::Value& json) {
    return EnrollUserRequestDto{
        .user_id = json["user_id"].As<std::int64_t>(0),
    };
}

inline userver::formats::json::Value ToJson(const EnrollmentResponseDto& dto) {
    userver::formats::json::ValueBuilder builder;

    builder["status"] = dto.status;
    builder["user_id"] = dto.user_id;
    builder["course_id"] = dto.course_id;
    
    return builder.ExtractValue();
}

}  // namespace lms_service::dto