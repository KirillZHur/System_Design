#pragma once

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>

#include "../../storage/postgres_storage.hpp"

namespace lms_service::views::lessons {

class ListLessonsHandler final
    : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-list-lessons";

  ListLessonsHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  lms_service::storage::PostgresStorage storage_;
};

}  // namespace lms_service::views::lessonsы