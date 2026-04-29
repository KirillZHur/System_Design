#include "add_lesson_handler.hpp"

#include <string>

#include <userver/server/http/http_status.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../dto/lesson_dto.hpp"
#include "../../storage/mongo_storage.hpp"

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
      storage_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()),
      pg_cluster_(
          context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()) {}

userver::formats::json::Value AddLessonHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {

  const auto dto = lms_service::dto::ParseAddLessonRequest(request_json);

  const auto course_id_str = request.GetPathArg("course_id");
  if (course_id_str.empty()) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("course_id is required");
  }

  const auto course_id = std::stoll(course_id_str);

  if (dto.title.empty() || dto.content.empty() || dto.position <= 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields title, content, position are required");
  }

  const auto created = storage_.AddLesson({
      .id = 0,
      .course_id = course_id,
      .title = dto.title,
      .content = dto.content,
      .position = dto.position,
  });

  pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "INSERT INTO lessons (course_id, title, content, position) "
      "VALUES ($1, $2, $3, $4)",
      created.course_id,
      created.title,
      created.content,
      created.position
  );

  request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
  return lms_service::dto::ToJson(
      lms_service::dto::ToLessonResponseDto(created));
}

}  // namespace lms_service::views::lessons