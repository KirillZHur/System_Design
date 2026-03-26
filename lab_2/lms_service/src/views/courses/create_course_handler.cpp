#include "create_course_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/course_dto.hpp"
#include "../../models/course.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::courses {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

CreateCourseHandler::CreateCourseHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value CreateCourseHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto dto = lms_service::dto::ParseCreateCourseRequest(request_json);

  if (dto.title.empty() || dto.description.empty() || dto.teacher_id <= 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse(
        "Fields title, description, teacher_id are required");
  }

  const auto teacher = storage_.GetUserById(dto.teacher_id);
  if (!teacher) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("Teacher not found");
  }

  const auto created = storage_.CreateCourse({
      .id = 0,
      .title = dto.title,
      .description = dto.description,
      .teacher_id = dto.teacher_id,
  });

  request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
  return lms_service::dto::ToJson(
      lms_service::dto::ToCourseResponseDto(*created));
}

}  // namespace lms_service::views::courses