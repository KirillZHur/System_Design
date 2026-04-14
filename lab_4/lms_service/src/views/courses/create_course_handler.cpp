#include "create_course_handler.hpp"

#include <string>

#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/mongo/component.hpp>

#include "../../dto/course_dto.hpp"
#include "../../storage/postgres_storage.hpp"
#include "../../storage/mongo_storage.hpp"

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
      storage_(context.FindComponent<userver::components::Mongo>("mongo-db")
                   .GetPool()),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()) {}

userver::formats::json::Value CreateCourseHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {

  const auto dto = lms_service::dto::ParseCreateCourseRequest(request_json);

  if (dto.title.empty() || dto.description.empty() || dto.teacher_id == 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields title, description, teacher_id are required");
  }

  const auto created = storage_.CreateCourse({
      .id = 0,
      .title = dto.title,
      .description = dto.description,
      .teacher_id = dto.teacher_id,
  });

  pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "INSERT INTO courses (id, title, description, teacher_id) VALUES ($1, $2, $3, $4)",
      created.id,
      created.title,
      created.description,
      created.teacher_id
  );

  request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);

  return lms_service::dto::ToJson(
      lms_service::dto::ToCourseResponseDto(created));
}

}  // namespace lms_service::views::courses