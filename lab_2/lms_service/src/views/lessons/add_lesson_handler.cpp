#include "add_lesson_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/lesson_dto.hpp"
#include "../../models/lesson.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::lessons {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

AddLessonHandler::AddLessonHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value AddLessonHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto course_id = std::stoll(std::string{request.GetPathArg("course_id")});
  const auto dto = lms_service::dto::ParseAddLessonRequest(request_json);

  if (dto.title.empty() || dto.content.empty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields title and content are required");
  }

  const auto course = storage_.GetCourseById(course_id);
  if (!course) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("Course not found");
  }

  const auto created = storage_.AddLesson({
      .id = 0,
      .course_id = course_id,
      .title = dto.title,
      .content = dto.content,
  });

  if (!created) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("Course not found");
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
  return lms_service::dto::ToJson(
      lms_service::dto::ToLessonResponseDto(*created));
}

}  // namespace lms_service::views::lessons