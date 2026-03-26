#include "list_courses_handler.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>

#include "../../dto/course_dto.hpp"
#include "../../storage/in_memory_storage.hpp"

namespace lms_service::views::courses {

ListCoursesHandler::ListCoursesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<lms_service::storage::InMemoryStorageComponent>()
                   .GetStorage()) {}

userver::formats::json::Value ListCoursesHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto courses = storage_.GetAllCourses();

  userver::formats::json::ValueBuilder response(
      userver::formats::common::Type::kArray);

  for (const auto& course : courses) {
    response.PushBack(lms_service::dto::ToJson(
        lms_service::dto::ToCourseResponseDto(course)));
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return response.ExtractValue();
}

}  // namespace lms_service::views::courses