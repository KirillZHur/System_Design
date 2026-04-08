#include "list_courses_handler.hpp"

#include <userver/formats/common/type.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../../dto/course_dto.hpp"
#include "../../storage/postgres_storage.hpp"

namespace lms_service::views::courses {

ListCoursesHandler::ListCoursesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<userver::components::Postgres>("postgres-db")
                   .GetCluster()) {}

userver::formats::json::Value ListCoursesHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto courses = storage_.GetAllCourses();

  userver::formats::json::ValueBuilder result(
      userver::formats::common::Type::kArray);

  for (const auto& course : courses) {
    result.PushBack(
        lms_service::dto::ToJson(lms_service::dto::ToCourseResponseDto(course)));
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return result.ExtractValue();
}

}  // namespace lms_service::views::courses