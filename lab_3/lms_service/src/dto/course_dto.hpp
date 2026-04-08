#pragma once

#include <cstdint>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

#include "../models/course.hpp"

namespace lms_service::dto {

struct CreateCourseRequestDto {
    std::string title;
    std::string description;
    std::int64_t teacher_id{0};
};

struct CourseResponseDto {
    std::int64_t id{0};
    std::string title;
    std::string description;
    std::int64_t teacher_id{0};
};

inline CreateCourseRequestDto ParseCreateCourseRequest(const userver::formats::json::Value& json) {
    return CreateCourseRequestDto{
        .title = json["title"].As<std::string>(""),
        .description = json["description"].As<std::string>(""),
        .teacher_id = json["teacher_id"].As<std::int64_t>(0),
    };
}

inline CourseResponseDto ToCourseResponseDto(const lms_service::models::Course& course) {
    return CourseResponseDto{
        .id = course.id,
        .title = course.title,
        .description = course.description,
        .teacher_id = course.teacher_id,
    };
}

inline userver::formats::json::Value ToJson(const CourseResponseDto& dto) {
    userver::formats::json::ValueBuilder builder;

    builder["id"] = dto.id;
    builder["title"] = dto.title;
    builder["description"] = dto.description;
    builder["teacher_id"] = dto.teacher_id;
    
    return builder.ExtractValue();
}

}  // namespace lms_service::dto