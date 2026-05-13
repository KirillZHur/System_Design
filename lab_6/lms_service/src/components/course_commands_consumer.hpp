#pragma once

#include <userver/components/component_base.hpp>
#include <userver/kafka/consumer_scope.hpp>
#include <userver/storages/mongo/pool.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace lms_service::components {

class CourseCommandsConsumer final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "course-commands-consumer";

  CourseCommandsConsumer(
      const userver::components::ComponentConfig& config,
      const userver::components::ComponentContext& context);

 private:
  void Consume(userver::kafka::MessageBatchView messages) const;

  bool IsMessageProcessed(const std::string& event_id) const;
  void MarkMessageProcessed(const std::string& event_id, const std::string& event_type) const;
  void SaveCourse(const userver::formats::json::Value& payload) const;

  userver::storages::postgres::ClusterPtr pg_cluster_;
  userver::storages::mongo::PoolPtr mongo_pool_;

  // ConsumerScope должен быть последним полем.
  userver::kafka::ConsumerScope consumer_;
};

}  // namespace lms_service::components