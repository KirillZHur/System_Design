#include "enroll_user_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../storage/postgres_storage.hpp"

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
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()) {}

userver::formats::json::Value EnrollUserHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto course_id_str = request.GetPathArg("course_id");
  const auto user_id = request_json["user_id"].As<std::int64_t>(0);

  if (course_id_str.empty() || user_id == 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields user_id and course_id are required");
  }

  const auto course_id = std::stoll(course_id_str);

  storage_.EnrollUserToCourse({
      .user_id = user_id,
      .course_id = course_id,
  });

  userver::formats::json::ValueBuilder builder;
  builder["status"] = "enrolled";
  builder["user_id"] = user_id;
  builder["course_id"] = course_id;

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return builder.ExtractValue();
}

}  // namespace lms_service::views::enrollments