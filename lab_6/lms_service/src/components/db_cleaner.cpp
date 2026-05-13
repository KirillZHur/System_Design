#include "db_cleaner.hpp"

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace lms_service::components {

DbCleaner::DbCleaner(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  const bool is_testing = config["is-testing"].As<bool>(false);

  if (!is_testing) {
    return;
  }

  auto pg = context
                .FindComponent<userver::components::Postgres>("postgres-db")
                .GetCluster();

  pg->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      R"(
        TRUNCATE TABLE
          lesson_progress,
          course_enrollments,
          lessons,
          courses,
          users
        RESTART IDENTITY CASCADE
      )");
}

userver::yaml_config::Schema DbCleaner::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(
      R"(
type: object
description: database cleaner for tests
additionalProperties: false
properties:
    is-testing:
        type: boolean
        description: run cleanup on startup in tests
)");
}

}  // namespace lms_service::components