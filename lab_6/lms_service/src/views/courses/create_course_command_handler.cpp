#include "create_course_command_handler.hpp"

#include <string>

#include <userver/formats/json/serialize.hpp>
#include <userver/kafka/producer_component.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/redis/component.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>

#include "../../dto/course_dto.hpp"

namespace lms_service::views::courses {

namespace {

constexpr std::string_view kCourseCommandsTopic = "lms.course.commands";

userver::formats::json::Value MakeErrorResponse(const std::string& message) {
  userver::formats::json::ValueBuilder builder;
  builder["error"] = message;
  return builder.ExtractValue();
}

userver::formats::json::Value MakeAcceptedResponse(
    const std::string& event_id,
    std::string_view topic) {
  userver::formats::json::ValueBuilder builder;
  builder["status"] = "accepted";
  builder["event_id"] = event_id;
  builder["topic"] = std::string{topic};
  return builder.ExtractValue();
}

}  // namespace

CreateCourseCommandHandler::CreateCourseCommandHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      producer_(context
                    .FindComponent<userver::kafka::ProducerComponent>(
                        "kafka-producer")
                    .GetProducer()),
      redis_client_(context
                        .FindComponent<userver::components::Redis>("redis")
                        .GetClient("cache-db")) {}

userver::formats::json::Value
CreateCourseCommandHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto dto = lms_service::dto::ParseCreateCourseRequest(request_json);

  if (dto.title.empty() || dto.description.empty() || dto.teacher_id == 0) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
    return MakeErrorResponse("Fields title, description, teacher_id are required");
  }

  const auto event_id = userver::utils::generators::GenerateUuid();

  userver::formats::json::ValueBuilder message;
  message["event_id"] = event_id;
  message["event_type"] = "CourseCreateRequested";
  message["occurred_at"] =
      userver::utils::datetime::Timestring(userver::utils::datetime::Now());
  message["payload"]["title"] = dto.title;
  message["payload"]["description"] = dto.description;
  message["payload"]["teacher_id"] = dto.teacher_id;

  const auto message_json =
      userver::formats::json::ToString(message.ExtractValue());

  const auto redis_key = "course:command:" + event_id;

  redis_client_
      ->Set(
          redis_key,
          message_json,
          std::chrono::seconds{3600},
          userver::storages::redis::CommandControl{})
      .Get();

  producer_.Send(
      std::string{kCourseCommandsTopic},
      event_id,
      message_json);

  request.SetResponseStatus(userver::server::http::HttpStatus::kAccepted);

  return MakeAcceptedResponse(event_id, kCourseCommandsTopic);
}

}  // namespace lms_service::views::courses