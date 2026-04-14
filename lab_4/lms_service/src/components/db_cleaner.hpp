#pragma once

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/schema.hpp>

namespace lms_service::components {

class DbCleaner final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "db-cleaner";

  DbCleaner(const userver::components::ComponentConfig& config,
            const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();
};

}  // namespace lms_service::components