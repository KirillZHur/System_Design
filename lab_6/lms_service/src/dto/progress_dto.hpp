#pragma once

#include <cstdint>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace lms_service::dto {

struct MarkProgressRequestDto {
    std::int64_t user_id{0};
    bool completed{false};
};

struct ProgressResponseDto {
    std::string status;
    std::int64_t user_id{0};
    std::int64_t lesson_id{0};
    bool completed{false};
};

inline MarkProgressRequestDto ParseMarkProgressRequest(const userver::formats::json::Value& json) {
    return MarkProgressRequestDto{
        .user_id = json["user_id"].As<std::int64_t>(0),
        .completed = json["completed"].As<bool>(false),
    };
}

inline userver::formats::json::Value ToJson(const ProgressResponseDto& dto) {
    userver::formats::json::ValueBuilder builder;

    builder["status"] = dto.status;
    builder["user_id"] = dto.user_id;
    builder["lesson_id"] = dto.lesson_id;
    builder["completed"] = dto.completed;
    
    return builder.ExtractValue();
}

}  // namespace lms_service::dto