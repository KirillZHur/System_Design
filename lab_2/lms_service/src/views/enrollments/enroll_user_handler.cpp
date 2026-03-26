#include "enroll_user_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/enrollment_dto.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::enrollments {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

EnrollUserHandler::EnrollUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value EnrollUserHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto course_id = std::stoll(std::string{request.GetPathArg("course_id")});
  const auto dto = lms_service::dto::ParseEnrollUserRequest(request_json);

  if (dto.user_id <= 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Field user_id is required");
  }

  const auto user = storage_.GetUserById(dto.user_id);
  if (!user) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("User not found");
  }

  const auto course = storage_.GetCourseById(course_id);
  if (!course) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("Course not found");
  }

  if (storage_.IsUserEnrolled(dto.user_id, course_id)) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
    return MakeErrorResponse("User already enrolled");
  }

  const bool enrolled = storage_.EnrollUser(dto.user_id, course_id);
  if (!enrolled) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Enrollment failed");
  }

  const lms_service::dto::EnrollmentResponseDto response_dto{
      .user_id = dto.user_id,
      .course_id = course_id,
      .status = "enrolled",
  };

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return lms_service::dto::ToJson(response_dto);
}

}  // namespace lms_service::views::enrollments