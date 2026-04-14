#include "list_lessons_handler.hpp"

#include <userver/formats/common/type.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/mongo/component.hpp>

#include "../../dto/lesson_dto.hpp"
#include "../../storage/mongo_storage.hpp"

namespace lms_service::views::lessons {

ListLessonsHandler::ListLessonsHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<userver::components::Mongo>("mongo-db")
             .GetPool()) {}

userver::formats::json::Value ListLessonsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto course_id_str = request.GetPathArg("course_id");
  const auto course_id = std::stoll(course_id_str);

  const auto lessons = storage_.GetLessonsByCourseId(course_id);

  userver::formats::json::ValueBuilder result(
      userver::formats::common::Type::kArray);

  for (const auto& lesson : lessons) {
    result.PushBack(
        lms_service::dto::ToJson(lms_service::dto::ToLessonResponseDto(lesson)));
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
  return result.ExtractValue();
}

}  // namespace lms_service::views::lessons