#include "course_commands_consumer.hpp"

#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/kafka/consumer_component.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/options.hpp>
#include <userver/storages/mongo/operations.hpp>
#include <userver/storages/postgres/component.hpp>

#include <vector>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/utils/datetime.hpp>

namespace lms_service::components {

CourseCommandsConsumer::CourseCommandsConsumer(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pg_cluster_(
          context
              .FindComponent<userver::components::Postgres>("postgres-db")
              .GetCluster()),
      mongo_pool_(
          context
              .FindComponent<userver::components::Mongo>("mongo-db")
              .GetPool()),
      consumer_(
          context
              .FindComponent<userver::kafka::ConsumerComponent>("kafka-consumer")
              .GetConsumer()) {
  consumer_.Start([this](userver::kafka::MessageBatchView messages) {
    Consume(messages);
    consumer_.AsyncCommit();
  });
}

bool CourseCommandsConsumer::IsMessageProcessed(
    const std::string& event_id) const {
  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "SELECT 1 FROM processed_kafka_messages WHERE event_id = $1",
      event_id);

  return !result.IsEmpty();
}

void CourseCommandsConsumer::MarkMessageProcessed(
    const std::string& event_id,
    const std::string& event_type) const {
  pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "INSERT INTO processed_kafka_messages(event_id, event_type) "
      "VALUES($1, $2) "
      "ON CONFLICT(event_id) DO NOTHING",
      event_id,
      event_type);
}

void CourseCommandsConsumer::SaveCourse(
    const userver::formats::json::Value& payload) const {
  const auto title = payload["title"].As<std::string>();
  const auto description = payload["description"].As<std::string>();
  const auto teacher_id = payload["teacher_id"].As<int>();

  const auto result = pg_cluster_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      "INSERT INTO courses(title, description, teacher_id) "
      "VALUES($1, $2, $3) "
      "RETURNING id",
      title,
      description,
      teacher_id);

  const auto course_id = result.AsSingleRow<int>();

  userver::formats::bson::ValueBuilder course_doc;
  course_doc["_id"] = static_cast<std::int64_t>(course_id);
  course_doc["title"] = title;
  course_doc["description"] = description;
  course_doc["teacher_id"] = static_cast<std::int64_t>(teacher_id);
  course_doc["created_at"] = std::chrono::system_clock::now();

  userver::formats::bson::ValueBuilder lessons;
  lessons.Resize(0);
  course_doc["lessons"] = lessons.ExtractValue();

  mongo_pool_->GetCollection("courses").InsertOne(course_doc.ExtractValue());
}

void CourseCommandsConsumer::Consume(
    userver::kafka::MessageBatchView messages) const {
  for (const auto& message : messages) {
    try {
      const auto event = userver::formats::json::FromString(message.GetPayload());

      const auto event_id = event["event_id"].As<std::string>();
      const auto event_type = event["event_type"].As<std::string>();

      if (event_type != "CourseCreateRequested") {
        LOG_WARNING() << "Unsupported event_type=" << event_type;
        continue;
      }

      if (IsMessageProcessed(event_id)) {
        LOG_INFO() << "Kafka message already processed: " << event_id;
        continue;
      }

      SaveCourse(event["payload"]);
      MarkMessageProcessed(event_id, event_type);

      LOG_INFO() << "CourseCreateRequested processed, event_id=" << event_id;
    } catch (const std::exception& ex) {
      LOG_ERROR() << "Failed to process Kafka message: " << ex.what();
      throw;
    }
  }
}

}  // namespace lms_service::components