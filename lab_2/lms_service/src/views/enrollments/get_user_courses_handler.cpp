#include "get_user_courses_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/course_dto.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::enrollments {

namespace {

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

}  // namespace

GetUserCoursesHandler::GetUserCoursesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value GetUserCoursesHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto user_id = std::stoll(std::string{request.GetPathArg("user_id")});

  const auto user = storage_.GetUserById(user_id);
  if (!user) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
    return MakeErrorResponse("User not found");
  }

  const auto courses = storage_.GetCoursesByUserId(user_id);

  userver::formats::json::ValueBuilder response(
      userver::formats::common::Type::kArray);

  for (const auto& course : courses) {
    response.PushBack(lms_service::dto::ToJson(
        lms_service::dto::ToCourseResponseDto(course)));
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return response.ExtractValue();
}

}  // namespace lms_service::views::enrollments