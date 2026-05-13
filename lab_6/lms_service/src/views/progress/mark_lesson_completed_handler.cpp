#include "mark_lesson_completed_handler.hpp"

#include <string>

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../storage/postgres_storage.hpp"

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
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()) {}

userver::formats::json::Value MarkLessonCompletedHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto lesson_id_str = request.GetPathArg("lesson_id");
  const auto user_id = request_json["user_id"].As<std::int64_t>(0);
  const auto completed = request_json["completed"].As<bool>(false);

  if (lesson_id_str.empty() || user_id == 0 || !completed) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields user_id, lesson_id and completed=true are required");
  }

  const auto lesson_id = std::stoll(lesson_id_str);

  storage_.MarkLessonCompleted({
      .user_id = user_id,
      .lesson_id = lesson_id,
      .completed = true,
  });

  userver::formats::json::ValueBuilder builder;
  builder["status"] = "completed";
  builder["user_id"] = user_id;
  builder["lesson_id"] = lesson_id;
  builder["completed"] = true;

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return builder.ExtractValue();
}

}  // namespace lms_service::views::progress