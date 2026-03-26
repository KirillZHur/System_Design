#include "list_lessons_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/lesson_dto.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::lessons {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

ListLessonsHandler::ListLessonsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value ListLessonsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto course_id = std::stoll(std::string{request.GetPathArg("course_id")});

  const auto course = storage_.GetCourseById(course_id);
  if (!course) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("Course not found");
  }

  const auto lessons = storage_.GetLessonsByCourseId(course_id);

  userver::formats::json::ValueBuilder response(
      userver::formats::common::Type::kArray);

  for (const auto& lesson : lessons) {
    response.PushBack(lms_service::dto::ToJson(
        lms_service::dto::ToLessonResponseDto(lesson)));
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return response.ExtractValue();
}

}  // namespace lms_service::views::lessons