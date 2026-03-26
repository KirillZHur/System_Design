#pragma once

#include <cstdint>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

#include "../models/lesson.hpp"

namespace lms_service::dto {

struct AddLessonRequestDto {
    std::string title;
    std::string content;
};

struct LessonResponseDto {
    std::int64_t id{0};
    std::int64_t course_id{0};
    std::string title;
    std::string content;
};

inline AddLessonRequestDto ParseAddLessonRequest(const userver::formats::json::Value& json) {
    return AddLessonRequestDto{
        .title = json["title"].As<std::string>(""),
        .content = json["content"].As<std::string>(""),
    };
}

inline LessonResponseDto ToLessonResponseDto(const lms_service::models::Lesson& lesson) {
    return LessonResponseDto{
        .id = lesson.id,
        .course_id = lesson.course_id,
        .title = lesson.title,
        .content = lesson.content,
    };
}

inline userver::formats::json::Value ToJson(const LessonResponseDto& dto) {
    userver::formats::json::ValueBuilder builder;

    builder["id"] = dto.id;
    builder["course_id"] = dto.course_id;
    builder["title"] = dto.title;
    builder["content"] = dto.content;
    
    return builder.ExtractValue();
}

}  // namespace lms_service::dto