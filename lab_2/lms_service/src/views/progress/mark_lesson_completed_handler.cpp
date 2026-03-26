#include "mark_lesson_completed_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/progress_dto.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::progress {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

MarkLessonCompletedHandler::MarkLessonCompletedHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value MarkLessonCompletedHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto lesson_id = std::stoll(std::string{request.GetPathArg("lesson_id")});
  const auto dto = lms_service::dto::ParseMarkProgressRequest(request_json);

  if (dto.user_id <= 0 || !dto.completed) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse(
        "Fields user_id and completed=true are required");
  }

  const auto user = storage_.GetUserById(dto.user_id);
  if (!user) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("User not found");
  }

  const auto lesson = storage_.GetLessonById(lesson_id);
  if (!lesson) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("Lesson not found");
  }

  const bool marked = storage_.MarkLessonCompleted(dto.user_id, lesson_id);
  if (!marked) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse(
        "User is not enrolled in this course or progress update failed");
  }

  const lms_service::dto::ProgressResponseDto response_dto{
      .status = "completed",
      .user_id = dto.user_id,
      .lesson_id = lesson_id,
      .completed = true,
  };

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return lms_service::dto::ToJson(response_dto);
}

}  // namespace lms_service::views::progress